#pragma once

#include "../Effect.h"

namespace Effects
{
    uint8_t fade1(Effect::Context &ctx)
    {
        // Настройка таймингов (в миллисекундах)
        const uint16_t FADE_DURATION = 400; // Сколько времени разгорается ОДНА ступень
        const uint16_t STEP_DELAY = 150;    // Задержка между стартом соседних ступеней

        // Определяем физический индекс ступени с учетом направления движения
        // Если идем вниз (dir == -1), то верхняя ступень (count-1) загорается первой
        uint8_t physicalStep = ctx.stepIdx;
        if (ctx.dir == -1)
        {
            // ctx.effect->count доступен, если добавить геттер, либо захардкодить 16:
            physicalStep = 16 - 1 - ctx.stepIdx;
        }

        // Вычисляем, в какой момент времени (мс) должна начать движение эта конкретная ступень
        uint32_t stepStartTime = physicalStep * STEP_DELAY;

        // Если общее время эффекта еще не дошло до старта этой ступени
        if (ctx.dt < stepStartTime)
        {
            return ctx.isLightOut ? 0xFF : 0x00; // Если тухнет — еще горит, если загорается — еще выключена
        }

        // Сколько времени конкретно эта ступень уже находится в состоянии анимации
        uint32_t localDt = ctx.dt - stepStartTime;

        // Если ступень уже полностью закончила анимацию
        if (localDt >= FADE_DURATION)
        {
            // Важно: не вызывайте здесь ctx.effect->finish(), так как этот код
            // срабатывает для КАЖДОЙ ступени отдельно. Перенесите finish в Effect::update!
            return ctx.isLightOut ? 0x00 : 0xFF;
        }

        // Плавный расчет яркости (0..255) без грубого деления на 20
        // Используем long long / float для исключения переполнения при расчете
        uint8_t brightness = (localDt * 255) / FADE_DURATION;

        if (ctx.isLightOut)
        {
            return 255 - brightness; // Затухание
        }
        else
        {
            return brightness; // Разгорание
        }
    }

    
    uint8_t runningFireWithTail(Effect::Context &ctx)
    {
        // Your pre-defined decay array (9 steps of fading)
        static const uint8_t fade[] = {255, 180, 50, 10, 0};
        static const uint8_t FADE_LENGTH = sizeof(fade) / sizeof(fade[0]); // Equals 9

        const uint16_t SPEED_MS = 100; // Time (ms) it takes for the fire to move to the next step
        const uint8_t TOTAL_STEPS = 16;

        // 1. Calculate the current position of the fire's head based on time
        // Every SPEED_MS, the head advances by 1 step.
        uint8_t headPos = ctx.dt / SPEED_MS;

        // Determine the fire direction
        // If dir == -1, the fire moves from bottom to top (inverting the head position)
        if (ctx.dir == -1)
        {
            // Head starts at 15 and goes down to 0 (and below into negative space)
            if (headPos > TOTAL_STEPS + FADE_LENGTH)
                return 0; // Effect finished
            headPos = (TOTAL_STEPS - 1) - headPos;
        }

        // 2. Calculate the distance between the current step and the moving head
        int8_t distance = 0;

        if (ctx.dir != -1)
        {
            // Moving DOWN (0 -> 15): Tail is BEHIND the head (headPos >= ctx.stepIdx)
            distance = headPos - ctx.stepIdx;
        }
        else
        {
            // Moving UP (15 -> 0): Tail is BEHIND the head (headPos <= ctx.stepIdx)
            distance = ctx.stepIdx - headPos;
        }

        // 3. Bound checking and array mapping
        // If distance is negative, the fire head hasn't reached this step yet.
        if (distance < 0)
        {
            return 0;
        }

        // If the distance is larger than our array, the tail has already passed this step.
        if (distance >= FADE_LENGTH)
        {
            return 0;
        }

        // 4. Return the exact brightness from the array based on pure integer distance
        return fade[distance];
    }

    uint8_t tetris1(Effect::Context &ctx)
    {
        const uint8_t TOTAL_STEPS = 16;
        const uint16_t FLY_DELAY = 60; // Скорость полета блока (мс на одну ступень)

        // --- РЕЖИМ 1: ВКЛЮЧЕНИЕ (Заполнение тетриса сверху вниз) ---
        if (!ctx.isLightOut)
        {
            uint32_t accumulatedTime = 0;

            // Просчитываем падение каждого блока по очереди
            for (uint8_t targetStep = 0; targetStep < TOTAL_STEPS; targetStep++)
            {
                // Физическое дно для текущего блока (тетрис заполняется снизу вверх: 15, 14, 13...)
                uint8_t floorStep = TOTAL_STEPS - 1 - targetStep;

                // Сколько времени летит этот конкретный блок до своего «пола»
                uint32_t flyDuration = (floorStep + 1) * FLY_DELAY;

                // Если общее время анимации еще не дошло до старта этого блока
                if (ctx.dt < accumulatedTime)
                {
                    // Если этот блок уже упал ранее, текущая проверяемая ступень (ctx.stepIdx)
                    // может быть частью уже заполненного дна.
                    if (ctx.stepIdx > floorStep)
                        return 255;
                    return 0;
                }

                // Время полета текущего блока
                uint32_t currentFlyTime = ctx.dt - accumulatedTime;

                if (currentFlyTime < flyDuration)
                {
                    // Блок СЕЙЧАС в полете. Вычисляем его текущую позицию на лестнице
                    uint8_t currentBlockPos = currentFlyTime / FLY_DELAY;

                    // Если проверяемая ступень — это место, где сейчас летит блок
                    if (ctx.stepIdx == currentBlockPos)
                        return 255;

                    // Проверяем, не находится ли текущая ступень в уже упавшем «дне»
                    if (ctx.stepIdx > floorStep)
                        return 255;

                    return 0;
                }

                // Блок закончил полет и упал на floorStep
                accumulatedTime += flyDuration;
            }
            return 255; // Все блоки упали, лестница горит
        }

        // --- РЕЖИМ 2: ВЫКЛЮЧЕНИЕ (Блоки падают «в подвал» сверху вниз) ---
        else
        {
            uint32_t accumulatedTime = 0;

            for (uint8_t targetStep = 0; targetStep < TOTAL_STEPS; targetStep++)
            {
                // Блоки улетают начиная с самого нижнего (15), затем 14, 13...
                uint8_t disappearingStep = TOTAL_STEPS - 1 - targetStep;

                // Сколько ступеней блоку нужно пролететь вниз, чтобы скрыться (упасть в подвал)
                uint8_t distanceToBottom = TOTAL_STEPS - disappearingStep;
                uint32_t flyDuration = distanceToBottom * FLY_DELAY;

                if (ctx.dt < accumulatedTime)
                {
                    // Этот блок еще не начал падать, значит ступени выше disappearingStep еще горят
                    if (ctx.stepIdx <= disappearingStep)
                        return 255;
                    return 0;
                }

                uint32_t currentFlyTime = ctx.dt - accumulatedTime;

                if (currentFlyTime < flyDuration)
                {
                    // Блок СЕЙЧАС падает вниз. Считаем, на сколько ступеней он уже опустился
                    uint8_t dropOffset = currentFlyTime / FLY_DELAY;
                    uint8_t currentBlockPos = disappearingStep + dropOffset;

                    // Отрисовываем летящую вниз ступень
                    if (ctx.stepIdx == currentBlockPos)
                        return 255;

                    // Все ступени, которые выше улетающего блока (еще не тронутые), продолжают гореть
                    if (ctx.stepIdx < disappearingStep)
                        return 255;

                    return 0;
                }

                // Блок улетел окончательно
                accumulatedTime += flyDuration;
            }
            return 0; // Все блоки улетели, лестница погасла
        }
    }
}
namespace Effects
{
    // Смягченные тайминги полета: движение плавное, размеренное, без агрессивного ускорения
    static const uint16_t FLY_TIMINGS[] = {
        0,   90,  175, 255, 330, 400, 465, 526, 
        583, 636, 685, 730, 772, 810, 845, 877, 906
    };

    uint8_t tetrisCozyFire(Effect::Context &ctx)
    {
        const uint8_t TOTAL_STEPS = 16;

        // 1. Хронологический расчет активного блока
        uint32_t accumulatedTime = 0;
        uint8_t currentFallingBlockIdx = 255;
        uint8_t settledCount = 0;
        uint32_t activeBlockLocalDt = 0;

        for (uint8_t i = 0; i < TOTAL_STEPS; i++)
        {
            uint8_t stepsToFly = ctx.isLightOut ? (i + 1) : (TOTAL_STEPS - i);
            uint32_t blockDuration = FLY_TIMINGS[stepsToFly];

            if (ctx.dt >= accumulatedTime && ctx.dt < accumulatedTime + blockDuration) {
                currentFallingBlockIdx = i;
                activeBlockLocalDt = ctx.dt - accumulatedTime;
            }
            if (ctx.dt >= accumulatedTime + blockDuration) {
                settledCount++;
            }
            accumulatedTime += blockDuration;
        }

        // 2. Пространственная координация (учет направления ctx.dir)
        uint8_t currentStep = ctx.stepIdx;
        if (ctx.dir == -1) {
            currentStep = (TOTAL_STEPS - 1) - currentStep;
        }

        // 3. Статичный «стакан» (упавшие блоки горят мягким ровным светом)
        if (!ctx.isLightOut) {
            if (currentStep >= (TOTAL_STEPS - settledCount)) return 255;
        } else {
            if (currentStep < (TOTAL_STEPS - settledCount)) return 255;
        }

        // Если анимация завершена
        if (currentFallingBlockIdx == 255) {
            return !ctx.isLightOut ? 255 : 0;
        }

        // 4. Поиск позиции головы летящего блока
        uint8_t stepsToFly = ctx.isLightOut ? (currentFallingBlockIdx + 1) : (TOTAL_STEPS - currentFallingBlockIdx);
        
        uint8_t headPos = 255;
        uint32_t headStepStartTime = 0;

        for (uint8_t step = 0; step < stepsToFly; step++) {
            if (activeBlockLocalDt >= FLY_TIMINGS[step] && activeBlockLocalDt < FLY_TIMINGS[step + 1]) {
                headPos = step;
                headStepStartTime = FLY_TIMINGS[step];
                break;
            }
        }

        if (headPos == 255) return 0;

        // Корректируем позицию головы под физическую геометрию падения
        if (ctx.isLightOut) {
            uint8_t startPos = TOTAL_STEPS - 1 - settledCount;
            headPos = startPos + headPos;
        }

        // 5. Магия уюта: Динамический мягкий шлейф (Temporal Soft Tail)
        // Вместо фиксированных шагов массива, мы смотрим, КУДА летит блок относительно текущей ступени
        if (currentStep <= headPos) 
        {
            // Насколько текущая ступень отстает от головы блока в пространстве
            uint8_t distance = headPos - currentStep;

            if (distance == 0) {
                // Голова блока: плавно разгорается от 180 до 255 по мере движения внутри шага сетки
                uint32_t stepDuration = FLY_TIMINGS[headPos + 1] - headStepStartTime;
                uint32_t insideStepDt = activeBlockLocalDt - headStepStartTime;
                return 180 + ((insideStepDt * 75) / stepDuration);
            }
            
            // Хвост: вместо массива гасим яркость экспоненциально в зависимости от расстояния.
            // distance == 1 -> ~140, distance == 2 -> ~77, distance == 3 -> ~42...
            // Деление сдвигом (>> distance) дает идеальное органическое затухание для глаз.
            uint16_t tailBrightness = 255 >> distance; 

            // Дополнительно размываем стыки во времени (линейный блендинг между шагами)
            uint32_t stepDuration = FLY_TIMINGS[headPos + 1] - headStepStartTime;
            uint32_t insideStepDt = activeBlockLocalDt - headStepStartTime;
            uint16_t nextTailBrightness = 255 >> (distance + 1);

            // Мягко интерполируем яркость шлейфа во времени, убирая микро-дергания
            return tailBrightness - (((tailBrightness - nextTailBrightness) * insideStepDt) / stepDuration);
        }

        return 0;
    }
}

#include <avr/pgmspace.h>

namespace Effects
{
    // SOLID: Оставляем таблицу гаммы чистым инструментом интерполяции (0..255)
    // static const uint8_t PROGMEM gammaTable256[] = {
    //     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,
    //     1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
    //     3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,   6,
    //     6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  11,  11,
    //     11,  12,  12,  13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,
    //     20,  20,  21,  21,  22,  23,  23,  24,  25,  25,  26,  27,  28,  28,  29,  30,
    //     31,  32,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
    //     46,  47,  48,  49,  51,  52,  53,  54,  56,  57,  58,  60,  61,  62,  64,  65,
    //     67,  68,  70,  71,  73,  75,  76,  78,  80,  81,  83,  85,  87,  89,  91,  93,
    //     95,  97,  99,  101, 103, 105, 107, 109, 112, 114, 116, 118, 121, 123, 126, 128,
    //     131, 133, 136, 138, 141, 144, 147, 149, 152, 155, 158, 161, 164, 167, 170, 173,
    //     176, 179, 182, 186, 189, 192, 196, 199, 203, 206, 210, 213, 217, 221, 225, 229,
    //     233, 237, 241, 245, 249, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    //     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    //     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    //     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
    // };

    uint8_t cozyBreathing(Effect::Context &ctx)
    {
        // --- НАСТРОЙКИ УЮТА И ЯРКОСТИ ---
        const uint8_t BRIGHTNESS_MIN = 5;   // Нижняя точка (едва заметный ночной контур)
        const uint8_t BRIGHTNESS_MAX = 10;  // ТВОЙ ПОТОЛОК ЯРКОСТИ (измени это число под себя, например 20-40)
        const uint16_t CYCLE_DURATION = 8000; // Скорость дыхания (5 секунд на полный вдох-выдох)
        
        // Математический рабочий диапазон
        const uint8_t DYNAMIC_RANGE = BRIGHTNESS_MAX - BRIGHTNESS_MIN;

        // 1. Приведение времени к циклу 0..255 для внутренней математики формы волны
        uint16_t cycleTime = ctx.dt % CYCLE_DURATION;
        uint8_t t = ((uint32_t)cycleTime * 255) / CYCLE_DURATION;

        uint8_t waveForm = 0;

        // 2. Генерация плавной формы дыхания (нормализована в 0..255)
        if (t < 100) {
            // Вдох
            waveForm = ((uint16_t)t * 255) / 100;
        } 
        else if (t >= 100 && t < 135) {
            // Мягкая пауза на пике вдоха
            waveForm = 255;
        } 
        else {
            // Выдох
            uint8_t tRemaining = 255 - t;
            waveForm = ((uint16_t)tRemaining * 255) / 120;
        }

        // 3. Исправление логарифмического восприятия глаза через гамму
        uint8_t gammaCorrected = waveForm;//pgm_read_byte(&gammaTable256[waveForm]);

        // 4. DRY & Безопасное масштабирование под ТВОЙ диапазон яркости
        uint8_t targetBrightness = BRIGHTNESS_MIN + (((uint16_t)gammaCorrected * DYNAMIC_RANGE) >> 8);

        // 5. Плавный и мягкий уход в полную темноту при выключении системы
        if (ctx.isLightOut) {
            uint32_t fadeOutDuration = 600; 
            if (ctx.dt >= fadeOutDuration) return 0;
            return ((uint32_t)targetBrightness * (fadeOutDuration - ctx.dt)) / fadeOutDuration;
        }

        return targetBrightness;
    }
}
