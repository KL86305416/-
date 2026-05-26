#include "gray_app.h"

#include "gpio.h"
#include "gray_protocol.h"
#include "usart.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} GrayGpioPin;

typedef struct
{
  uint8_t sensor_bits;
  uint8_t transitions;
  uint8_t active_count;
  uint8_t left_count;
  uint8_t right_count;
  uint8_t track_type;
  uint8_t scene;
  uint8_t flags;
  int16_t center_err;
  int8_t turn;
} GrayAnalysisResult;

#define GRAY_SENSOR_COUNT              8U
#define GRAY_SENSOR_ACTIVE_IS_LOW      1U
#define GRAY_LED_ACTIVE_IS_LOW         1U
#define GRAY_LED_MIRROR_LEFT_RIGHT     1U
#define GRAY_STATUS_TX_PERIOD_MS      20U
#define GRAY_KEY_DEBOUNCE_MS          30U
#define GRAY_TURN_SCALE               2
#define GRAY_TURN_SOFT_THRESHOLD      10
#define GRAY_TURN_HARD_THRESHOLD      45
#define GRAY_TURN_HARD_MIN            60
#define GRAY_TURN_LOST_LINE_RECOVERY  25
#define GRAY_WIDE_BLACK_THRESHOLD     6U
#define GRAY_CENTER_MASK              0x18U
#define GRAY_LEFT_MASK                0x0FU
#define GRAY_RIGHT_MASK               0xF0U
#define GRAY_TX_BUFFER_SIZE           GRAY_RUNTIME_FRAME_LEN

static const GrayGpioPin g_sensor_inputs[GRAY_SENSOR_COUNT] =
{
  {GPIOA, GPIO_PIN_2},
  {GPIOA, GPIO_PIN_3},
  {GPIOA, GPIO_PIN_4},
  {GPIOA, GPIO_PIN_5},
  {GPIOB, GPIO_PIN_12},
  {GPIOB, GPIO_PIN_13},
  {GPIOB, GPIO_PIN_15},
  {GPIOA, GPIO_PIN_8}
};

static const GrayGpioPin g_sensor_leds[GRAY_SENSOR_COUNT] =
{
  {GPIOA, GPIO_PIN_15},
  {GPIOB, GPIO_PIN_3},
  {GPIOB, GPIO_PIN_4},
  {GPIOB, GPIO_PIN_5},
  {GPIOB, GPIO_PIN_6},
  {GPIOB, GPIO_PIN_7},
  {GPIOB, GPIO_PIN_8},
  {GPIOB, GPIO_PIN_9}
};

static uint32_t g_last_tx_tick = 0U;
static uint32_t g_last_key_tick = 0U;
static GPIO_PinState g_last_key_level = GPIO_PIN_SET;
static uint8_t g_stream_enabled = 1U;
static uint8_t g_tx_buffer[GRAY_TX_BUFFER_SIZE];
static int8_t g_last_valid_turn = 0;

static uint8_t GrayApp_ReadSensorBits(void)
{
  uint8_t sensor_bits = 0U;
  uint8_t index;

  for (index = 0U; index < GRAY_SENSOR_COUNT; ++index)
  {
    GPIO_PinState raw_level;
    uint8_t active;

    raw_level = HAL_GPIO_ReadPin(g_sensor_inputs[index].port, g_sensor_inputs[index].pin);
#if GRAY_SENSOR_ACTIVE_IS_LOW
    active = (raw_level == GPIO_PIN_RESET) ? 1U : 0U;
#else
    active = (raw_level == GPIO_PIN_SET) ? 1U : 0U;
#endif

    sensor_bits |= (uint8_t)(active << index);
  }

  return sensor_bits;
}

static void GrayApp_UpdateSensorLeds(uint8_t sensor_bits)
{
  uint8_t led_index;
  GPIO_PinState led_level;

  for (led_index = 0U; led_index < GRAY_SENSOR_COUNT; ++led_index)
  {
    uint8_t sensor_index = led_index;

#if GRAY_LED_MIRROR_LEFT_RIGHT
    sensor_index = (uint8_t)((GRAY_SENSOR_COUNT - 1U) - led_index);
#endif

    if ((sensor_bits & (uint8_t)(1U << sensor_index)) != 0U)
    {
#if GRAY_LED_ACTIVE_IS_LOW
      led_level = GPIO_PIN_RESET;
#else
      led_level = GPIO_PIN_SET;
#endif
    }
    else
    {
#if GRAY_LED_ACTIVE_IS_LOW
      led_level = GPIO_PIN_SET;
#else
      led_level = GPIO_PIN_RESET;
#endif
    }

    HAL_GPIO_WritePin(g_sensor_leds[led_index].port,
                      g_sensor_leds[led_index].pin,
                      led_level);
  }
}

static uint8_t GrayApp_CountTransitions(uint8_t sensor_bits)
{
  uint8_t transitions = 0U;
  uint8_t index;

  for (index = 0U; index < (GRAY_SENSOR_COUNT - 1U); ++index)
  {
    uint8_t current_bit;
    uint8_t next_bit;

    current_bit = (uint8_t)((sensor_bits >> index) & 0x01U);
    next_bit = (uint8_t)((sensor_bits >> (index + 1U)) & 0x01U);
    if (current_bit != next_bit)
    {
      transitions++;
    }
  }

  return transitions;
}

static uint8_t GrayApp_CountActiveSensors(uint8_t sensor_bits)
{
  uint8_t count = 0U;
  uint8_t index;

  for (index = 0U; index < GRAY_SENSOR_COUNT; ++index)
  {
    if ((sensor_bits & (uint8_t)(1U << index)) != 0U)
    {
      count++;
    }
  }

  return count;
}

static uint8_t GrayApp_CountBits(uint8_t value)
{
  uint8_t count = 0U;

  while (value != 0U)
  {
    count = (uint8_t)(count + (value & 0x01U));
    value >>= 1;
  }

  return count;
}

static int16_t GrayApp_ComputeCenterError(uint8_t sensor_bits)
{
  static const int16_t k_positions[GRAY_SENSOR_COUNT] = {-35, -25, -15, -5, 5, 15, 25, 35};
  int16_t weighted_sum = 0;
  uint8_t count = 0U;
  uint8_t index;

  for (index = 0U; index < GRAY_SENSOR_COUNT; ++index)
  {
    if ((sensor_bits & (uint8_t)(1U << index)) != 0U)
    {
      weighted_sum = (int16_t)(weighted_sum + k_positions[index]);
      count++;
    }
  }

  if (count == 0U)
  {
    return 0;
  }

  return (int16_t)(weighted_sum / (int16_t)count);
}

static int8_t GrayApp_ClampTurn(int16_t value)
{
  if (value > 100)
  {
    return 100;
  }

  if (value < -100)
  {
    return -100;
  }

  return (int8_t)value;
}

static int8_t GrayApp_ComputeTurn(int16_t center_err)
{
  return GrayApp_ClampTurn((int16_t)(center_err * GRAY_TURN_SCALE));
}

static uint8_t GrayApp_ComputeTrackType(uint8_t sensor_bits, uint8_t transitions, uint8_t active_count)
{
  if (active_count == 0U)
  {
    return GRAY_TRACK_NONE;
  }

  if (sensor_bits == 0xFFU)
  {
    return GRAY_TRACK_ALL_BLACK;
  }

  if ((sensor_bits & 0x01U) != 0U)
  {
    if ((sensor_bits & 0x80U) == 0U)
    {
      return GRAY_TRACK_LEFT_EDGE;
    }
  }

  if ((sensor_bits & 0x80U) != 0U)
  {
    if ((sensor_bits & 0x01U) == 0U)
    {
      return GRAY_TRACK_RIGHT_EDGE;
    }
  }

  if (transitions <= 2U)
  {
    return GRAY_TRACK_SINGLE_LINE;
  }

  if (active_count >= 6U)
  {
    return GRAY_TRACK_CROSS;
  }

  return GRAY_TRACK_AMBIGUOUS;
}

static uint8_t GrayApp_BuildRuntimeFlags(const GrayAnalysisResult *analysis)
{
  uint8_t flags = GRAY_RUNTIME_FLAG_VALID;

  if (analysis->active_count > 0U)
  {
    flags |= GRAY_RUNTIME_FLAG_LINE_FOUND;
  }

  if (analysis->left_count >= 2U)
  {
    flags |= GRAY_RUNTIME_FLAG_LEFT_FEATURE;
  }

  if (analysis->right_count >= 2U)
  {
    flags |= GRAY_RUNTIME_FLAG_RIGHT_FEATURE;
  }

  if ((analysis->sensor_bits & GRAY_CENTER_MASK) != 0U)
  {
    flags |= GRAY_RUNTIME_FLAG_CENTER_ON_LINE;
  }

  if (analysis->active_count >= GRAY_WIDE_BLACK_THRESHOLD)
  {
    flags |= GRAY_RUNTIME_FLAG_WIDE_BLACK;
  }

  if ((analysis->track_type == GRAY_TRACK_AMBIGUOUS) || (analysis->transitions >= 4U))
  {
    flags |= GRAY_RUNTIME_FLAG_UNSTABLE;
  }

  return flags;
}

static uint8_t GrayApp_ClassifyScene(const GrayAnalysisResult *analysis)
{
  uint8_t center_on_line;
  uint8_t left_feature;
  uint8_t right_feature;
  uint8_t wide_black;

  center_on_line = (uint8_t)((analysis->flags & GRAY_RUNTIME_FLAG_CENTER_ON_LINE) != 0U);
  left_feature = (uint8_t)((analysis->flags & GRAY_RUNTIME_FLAG_LEFT_FEATURE) != 0U);
  right_feature = (uint8_t)((analysis->flags & GRAY_RUNTIME_FLAG_RIGHT_FEATURE) != 0U);
  wide_black = (uint8_t)((analysis->flags & GRAY_RUNTIME_FLAG_WIDE_BLACK) != 0U);

  if (analysis->active_count == 0U)
  {
    return GRAY_SCENE_LINE_LOST;
  }

  if ((analysis->sensor_bits == 0xFFU) ||
      (wide_black != 0U && center_on_line != 0U &&
       analysis->left_count >= 3U && analysis->right_count >= 3U &&
       analysis->active_count >= 7U))
  {
    return GRAY_SCENE_STOP_BAR;
  }

  if (center_on_line != 0U &&
      left_feature != 0U &&
      right_feature != 0U &&
      analysis->active_count >= 5U)
  {
    return GRAY_SCENE_CROSS;
  }

  if (center_on_line == 0U &&
      left_feature != 0U &&
      right_feature != 0U &&
      analysis->active_count >= 4U)
  {
    return GRAY_SCENE_T_JUNCTION;
  }

  if (center_on_line != 0U &&
      left_feature != 0U &&
      right_feature == 0U &&
      analysis->active_count >= 4U)
  {
    return GRAY_SCENE_LEFT_BRANCH;
  }

  if (center_on_line != 0U &&
      right_feature != 0U &&
      left_feature == 0U &&
      analysis->active_count >= 4U)
  {
    return GRAY_SCENE_RIGHT_BRANCH;
  }

  if ((analysis->turn <= -GRAY_TURN_HARD_THRESHOLD) ||
      (analysis->left_count >= 3U && analysis->right_count == 0U && analysis->transitions <= 2U))
  {
    return GRAY_SCENE_HARD_LEFT;
  }

  if ((analysis->turn >= GRAY_TURN_HARD_THRESHOLD) ||
      (analysis->right_count >= 3U && analysis->left_count == 0U && analysis->transitions <= 2U))
  {
    return GRAY_SCENE_HARD_RIGHT;
  }

  if (analysis->turn <= -GRAY_TURN_SOFT_THRESHOLD)
  {
    return GRAY_SCENE_SOFT_LEFT;
  }

  if (analysis->turn >= GRAY_TURN_SOFT_THRESHOLD)
  {
    return GRAY_SCENE_SOFT_RIGHT;
  }

  if ((analysis->flags & GRAY_RUNTIME_FLAG_UNSTABLE) != 0U)
  {
    return GRAY_SCENE_AMBIGUOUS;
  }

  return GRAY_SCENE_STRAIGHT;
}

static int8_t GrayApp_AdjustTurnForScene(uint8_t scene, int8_t turn)
{
  switch (scene)
  {
    case GRAY_SCENE_HARD_LEFT:
      if (turn > -GRAY_TURN_HARD_MIN)
      {
        return -GRAY_TURN_HARD_MIN;
      }
      return turn;

    case GRAY_SCENE_HARD_RIGHT:
      if (turn < GRAY_TURN_HARD_MIN)
      {
        return GRAY_TURN_HARD_MIN;
      }
      return turn;

    case GRAY_SCENE_STOP_BAR:
      return 0;

    case GRAY_SCENE_LINE_LOST:
      if (g_last_valid_turn > 0)
      {
        return GRAY_TURN_LOST_LINE_RECOVERY;
      }
      if (g_last_valid_turn < 0)
      {
        return -GRAY_TURN_LOST_LINE_RECOVERY;
      }
      return 0;

    default:
      return turn;
  }
}

static void GrayApp_UpdateRuntimeOutput(const GrayAnalysisResult *analysis)
{
  GrayRuntimeFrame runtime_frame;

  runtime_frame.turn = analysis->turn;
  runtime_frame.scene = analysis->scene;
  runtime_frame.flags = analysis->flags;
  GrayProtocol_SetRuntimeOutput(&runtime_frame);
}

static void GrayApp_UpdateStatus(void)
{
  GrayAnalysisResult analysis;

  analysis.sensor_bits = GrayApp_ReadSensorBits();
  analysis.transitions = GrayApp_CountTransitions(analysis.sensor_bits);
  analysis.active_count = GrayApp_CountActiveSensors(analysis.sensor_bits);
  analysis.left_count = GrayApp_CountBits((uint8_t)(analysis.sensor_bits & GRAY_LEFT_MASK));
  analysis.right_count = GrayApp_CountBits((uint8_t)((analysis.sensor_bits & GRAY_RIGHT_MASK) >> 4));
  analysis.center_err = GrayApp_ComputeCenterError(analysis.sensor_bits);
  analysis.track_type = GrayApp_ComputeTrackType(analysis.sensor_bits,
                                                 analysis.transitions,
                                                 analysis.active_count);
  analysis.turn = GrayApp_ComputeTurn(analysis.center_err);
  analysis.flags = GrayApp_BuildRuntimeFlags(&analysis);
  analysis.scene = GrayApp_ClassifyScene(&analysis);
  analysis.turn = GrayApp_AdjustTurnForScene(analysis.scene, analysis.turn);

  if ((analysis.flags & GRAY_RUNTIME_FLAG_LINE_FOUND) != 0U &&
      analysis.scene != GRAY_SCENE_STOP_BAR &&
      analysis.scene != GRAY_SCENE_AMBIGUOUS)
  {
    g_last_valid_turn = analysis.turn;
  }

  GrayApp_UpdateSensorLeds(analysis.sensor_bits);
  GrayProtocol_SetSensorBits(analysis.sensor_bits);
  GrayProtocol_SetEdgeCount(analysis.transitions);
  GrayProtocol_SetTrackType(analysis.track_type);
  GrayProtocol_SetCenterError(analysis.center_err);
  GrayApp_UpdateRuntimeOutput(&analysis);
}

static void GrayApp_UpdateStreamIndicator(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (g_stream_enabled != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void GrayApp_HandleKey(void)
{
  GPIO_PinState current_level;
  uint32_t now_tick;

  current_level = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
  now_tick = HAL_GetTick();

  if ((current_level == GPIO_PIN_RESET) &&
      (g_last_key_level == GPIO_PIN_SET) &&
      ((now_tick - g_last_key_tick) >= GRAY_KEY_DEBOUNCE_MS))
  {
    g_last_key_tick = now_tick;
    g_stream_enabled ^= 1U;
    GrayProtocol_SetUartStreamEnabled(g_stream_enabled);
    GrayApp_UpdateStreamIndicator();
  }

  g_last_key_level = current_level;
}

static void GrayApp_SendRuntimeFrame(void)
{
  uint16_t frame_len;

  GrayProtocol_CommitFrame();
  frame_len = GrayProtocol_BuildRuntimeShortFrame(g_tx_buffer, GRAY_TX_BUFFER_SIZE);
  if (frame_len > 0U)
  {
    (void)HAL_UART_Transmit(&huart1, g_tx_buffer, frame_len, 10U);
  }
}

void GrayApp_Init(void)
{
  g_last_tx_tick = HAL_GetTick();
  g_last_key_tick = 0U;
  g_last_key_level = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
  g_stream_enabled = 1U;
  g_last_valid_turn = 0;

  GrayProtocol_SetUartStreamEnabled(g_stream_enabled);
  GrayApp_UpdateStatus();
  GrayApp_UpdateStreamIndicator();
  GrayApp_SendRuntimeFrame();
}

void GrayApp_Poll(void)
{
  uint32_t now_tick;

  GrayApp_HandleKey();
  GrayApp_UpdateStatus();

  now_tick = HAL_GetTick();
  if ((g_stream_enabled != 0U) && ((now_tick - g_last_tx_tick) >= GRAY_STATUS_TX_PERIOD_MS))
  {
    g_last_tx_tick = now_tick;
    GrayApp_SendRuntimeFrame();
  }
}
