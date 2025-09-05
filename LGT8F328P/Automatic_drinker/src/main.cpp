#include <Arduino.h>
/////////////////////////////////////////////////
// Automatic drinker v1.3 11.08.2025

// Улучшения:  - Добавлен PWM режим для насоса
//  По умолчанию поилка тестируется и влючает насос, если обнаружена вода.
//  По нажатию конпки включается и выключается насос и диод
//  Если воды нет, сразу включается мигание диодом и насос соответсвенно не работат
//
/////////////////////////////////////////////////

#define MOTOR 3        // мы сказали Arduino, что будем использовать слово MOTOR, но на самом деле это цифра 3
#define BUTTON 5       // тут кнопка
#define LEVEL_SENSOR A0       // датчик уровня
#define LED 13         // диод
#define PERIOD 1000    // период для таймера

////////////////////////////////////////////////////

uint32_t timer = 0;
bool noWater = true; // по умолчанию воды нет

boolean butt_count = 1;     // флажок — по умолчанию поднят = позволяет включать сразу, по присутствии воды
boolean butt;               // принимает текущее значение кнопки
unsigned long last_press;   // хранит время последнего нажатия

static String currentVersion = "version 1.3"


// Прототипы
void fadedLed();
void timDeb();
void button();
bool selfTest();

/* Первоначальая настройка */
void setup()
{

    // Конфигурация пинов
    pinMode(MOTOR, OUTPUT);
    pinMode(LEVEL, INPUT_PULLUP);
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(LED, OUTPUT);


    Serial.begin(9600); // инициализируем порт
    
}

/* Главная функция */
void loop()
{

    //////// обработка от датчика уровня воды ////////
    sensorValue = analogRead(LEVEL); // считываем показатели с датчика в переменную
    // типа защита от недостоверных показаний, отбрасываем показания до 100
    if (sensorValue > 100)
    {
        waterFlag = true; // вода есть
        button();
    }
    else
    {
        waterFlag = false;        // воды нет
        digitalWrite(MOTOR, LOW); // отключаем насос
        fadedLed();               // функция мигалки
    }
    ////////////////////////////////

#if (DEBUG == 1)
    timDeb();
#endif
}


// Первоначальный тест
bool selfTest()
{
// вывод в порт
    Serial.println("Automatic drinker "+ currentVersion);

    // Первоначальный тест диода
    digitalWrite(LED, 1);
    delay(2000);
    digitalWrite(LED, 0);
    delay(1000);
    Serial.println("\t LED TEST - OK");
}
//////// таймер + отправка в порт ////////
void timDeb()
{
    if (millis() - timer >= PERIOD)
    {
        // выводим в порт
        Serial.print("\t SensorValue = ");
        Serial.println(sensorValue);
        Serial.print("\t Water = ");
        if (waterFlag == true)
        {
            Serial.println("OK");
        }
        else
        {
            Serial.println("NULL");
        }
        Serial.print("\t Сlick conunt = ");
        Serial.println(butt_count);

        Serial.print("\t PowerTotal = ");
        Serial.println(pwrTotal);
#if (DEBUG_PWM == 1)
        // ТЕСТ работоспособности PWM путем изменения шага, шаг "костыльный" - т.к. итерация происходит со значением переменной PERIOD
        pwrTotal = pwrTotal + duty;
        if (duty <= 0 || duty >= 255)
        {
            duty = -duty;
        }
#endif

        // выводим в порт
        Serial.print(" Mode = ");
        switch (butt_count && waterFlag)
        {
        case 0:
            Serial.println("OFF");
            break;
        case 1:
            Serial.println("RUNNING");

            break;
        }

        do
        {
            timer += PERIOD;
            if (timer < PERIOD)
                break; // переполнение uint32_t
        } while (timer < millis() - PERIOD); // защита от пропуска шага
    }
}

//////// мигаем диодом ////////
void fadedLed()
{
    static uint32_t tmr;
    static bool flag; //

    uint32_t period;
    if (flag)
        period = 1000;
    else
        period = 1000;

    if (millis() - tmr >= period)
    {
        tmr = millis();
        digitalWrite(LED, !digitalRead(LED));
        flag = !flag;
    }
}

/////// отработка кнопки ////////
void button()
{
    butt = !digitalRead(BUTTON); // считывает текущее положение кнопки
    // Serial.println(butt);
    if (butt == 1 && butt_count == 0 && millis() - last_press > 200)
    { // последнее условие - кнопка перестает опрашиваться в течение 50 млсек
        // если кнопка нажата и если она не была нажата ранее
        butt_count = 1; // поднимаем флажок
        // Serial.println("BUTTON PRESSED");
        // digitalWrite(LED, LOW);
        last_press = millis();
    }

    if (butt == 1 && butt_count == 1 && millis() - last_press > 200)
    {
        butt_count = 0; // опускаем флаг
        last_press = millis();
    }
    switch (butt_count && waterFlag)
    {
    case 0:
        analogWrite(MOTOR, 0); // переход на аналоговый сингал (PWM)
        digitalWrite(LED, 0);

        break;
    case 1:
        analogWrite(MOTOR, POWERTOTAL); // переход на аналоговый сингал (PWM)
        digitalWrite(LED, 1);

        break;
    }
}