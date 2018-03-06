/*
 * Project particle_control
 * Description: example of how to use particle cloud functions and variables.
 * Author: Zhi Shen Yong
 * Date: March 5, 2018
 */

// RGB LED pin constants
const int redPin = A4;
const int greenPin = D0;
const int bluePin = D1;

// RGB LED analog values
int redValue = 255;
int greenValue = 255;
int blueValue = 255;

// RGB fade parameters
const int fadeInterval = 1000;
const int fadeDuration = 5000;
unsigned long startFade = 0;
int startRed;
int endRed;
int startGreen;
int endGreen;
int startBlue;
int endBlue;
Timer rgbFadeUpdateTimer(fadeInterval, updateRGB);
Timer rgbFadeTimer(fadeDuration, fadeEnd, true);

// button variables
const int buttonPin = D2;
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debouncePeriod = 50;

// initial light boolean variable
bool lightOn = true;

// rand
const String topic = "particle_control";

void setup() {

    Serial.begin(9600);

    // set RGB LED pins to OUTPUT
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    // set initial RGB values
    analogWrite(redPin, redValue);
    analogWrite(greenPin, greenValue);
    analogWrite(bluePin, blueValue);

    // set button pin to INPUT
    pinMode(buttonPin, INPUT_PULLUP);

    // declare Particle cloud functions
    // remmeber that cloud functinos must take in a String and return an int
    Particle.function("setLampOn", setLampOn);
    Particle.function("setLampColor", setLampColor);
    Particle.function("pushState", pushState);

    // declare Particle cloud variables
    Particle.variable("lightOn", lightOn);

}

void loop() {
    // read the current button state
    int reading = digitalRead(buttonPin);
    // filter out button state changes due to noise
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    // if same state is held for at least debouncePeriod
    if ((millis() - lastDebounceTime) > debouncePeriod) {
        // if buttonState has changed
        if (reading != buttonState) {
            buttonState = reading;
            // remember we are using INPUT_PULLUP
            // LOW happens when button is pushed
            if (buttonState == LOW) {
                if (lightOn == true) {
                    // if the light was initially on, we will turn it off
                    setLampOn("false");
                } else {
                    // if the light was initially off, we will turn it in
                    setLampOn("true");
                }
            }
        }
    }
    lastButtonState = reading;
}

int setLampOn(String value) {
    // set the global on/off variable
    if (value == "true") {
        lightOn = true;
    } else {
        lightOn = false;
    }
    // turn the lamp on or off depending on the argument
    if (lightOn == true) {
        analogWrite(redPin, redValue);
        analogWrite(greenPin, greenValue);
        analogWrite(bluePin, blueValue);
    } else {
        analogWrite(redPin, 0);
        analogWrite(greenPin, 0);
        analogWrite(bluePin, 0);
    }

    // update the state
    pushState("");
    return 0;

}

int setLampColor(String value) {

    // get position of commas
    int firstCommaIndex = value.indexOf(",");
    int secondCommaIndex = value.indexOf(",", firstCommaIndex + 1);

    // get substrings that represent integers
    String tempRed = value.substring(0, firstCommaIndex);
    String tempGreen = value.substring(firstCommaIndex + 1, secondCommaIndex);
    String tempBlue = value.substring(secondCommaIndex + 1);

    // call rgbFade
    rgbFade(redValue, tempRed.toInt(), greenValue, tempGreen.toInt(), blueValue, tempBlue.toInt());
    return 0;

}

void updateRGB() {
    unsigned long now = millis();
    double fraction = min((now - startFade)/(double)fadeDuration, 1.0);
    redValue = (int)(startRed + (endRed - startRed) * fraction);
    greenValue = (int)(startGreen + (endGreen - startGreen) * fraction);
    blueValue = (int)(startBlue + (endBlue - startBlue) * fraction);
    if (lightOn == true) {
        analogWrite(redPin, redValue);
        analogWrite(greenPin, greenValue);
        analogWrite(bluePin, blueValue);
    }
    pushState("");
}

void fadeEnd() {
    rgbFadeUpdateTimer.stop();
    redValue = endRed;
    greenValue = endGreen;
    blueValue = endBlue;
    if (lightOn == true) {
        analogWrite(redPin, redValue);
        analogWrite(greenPin, greenValue);
        analogWrite(bluePin, blueValue);
    }
    pushState("");
}

void rgbFade(int startRedPass, int endRedPass, int startGreenPass, int endGreenPass, int startBluePass, int endBluePass) {
    startFade = millis();
    startRed = startRedPass;
    endRed = endRedPass;
    startGreen = startGreenPass;
    endGreen = endGreenPass;
    startBlue = startBluePass;
    endBlue = endBluePass;
    rgbFadeUpdateTimer.start();
    rgbFadeTimer.start();
}

int pushState(String value) {

    // construct the state
    String state = "{";
    if (lightOn) {
        state += "\"powered\":true";
    } else {
        state += "\"powered\":false";
    }
    state += ", ";
    state += "\"r\":";
    state += redValue;
    state += ", ";
    state += "\"g\":";
    state += greenValue;
    state += ", ";
    state += "\"b\":";
    state += blueValue;
    state += "}";

    // print the state to serial monitor
    Serial.println("Publishing: ");
    Serial.println(state);

    // publish the state onto cloud
    Particle.publish(topic, state, 60, PRIVATE);
    return 0;
}
