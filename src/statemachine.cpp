//
// Created by Timo Stülten on 10.04.24.
//
#ifndef statemachine_h
#define statemachine_h

enum class State {
    WAIT,
    COLOR_DETECTION,
    COLOR_SORT,
    COLOR_DUMP,
    CHUTE_MOVE,
    ERROR_STATE
};
const char* toString(State state) {
    switch (state) {
        case State::WAIT: return "WAIT";
        case State::COLOR_DETECTION: return "COLOR_DETECTION";
        case State::COLOR_SORT: return "COLOR_SORT";
        case State::COLOR_DUMP: return "COLOR_DUMP";
        case State::CHUTE_MOVE: return "CHUTE_MOVE";
        case State::ERROR_STATE: return "ERROR_STATE";
        default: return "UNKNOWN_STATE";
    }
}

enum class Event {
    NONE,
    INITIALIZED,
    COLOR_NEW_SENSOR_VALUE,
    COLOR_DETECTED,
    COLOR_NOT_DETECTED,
    CHUTE_MOVED,
    ERROR
};

const char* toString(Event event) {
    switch (event) {
        case Event::NONE: return "NONE";
        case Event::INITIALIZED: return "INITIALIZED";
        case Event::COLOR_NEW_SENSOR_VALUE: return "COLOR_NEW_SENSOR_VALUE";
        case Event::COLOR_DETECTED: return "COLOR_DETECTED";
        case Event::COLOR_NOT_DETECTED: return "COLOR_NOT_DETECTED";
        case Event::CHUTE_MOVED: return "CHUTE_MOVED";
        case Event::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

State currentState = State::WAIT;

Event pendingEvent = Event::NONE;

Event setNextEvent(Event nextEvent) {
    Serial.print("New Event: ");
    Serial.println(toString(nextEvent));
    Event ret = pendingEvent;
    pendingEvent = nextEvent;
    return ret;
}

Event checkForEvent() {
    return pendingEvent;
}

void executeActionForState(State state) {
    Serial.print("New State: ");
    Serial.println(toString(state));
    switch (state) {
        case State::WAIT:
            // Action for WAIT state
            break;
        case State::COLOR_DETECTION:
            // Action for COLOR_DETECTION state
            break;
        case State::COLOR_SORT:
            // Action for COLOR_SORT state
            break;
        case State::COLOR_DUMP:
            // Action for COLOR_DUMP state
            break;
        case State::CHUTE_MOVE:
            // Action for CHUTE_MOVE state, then return to WAIT
            currentState = State::WAIT;
            break;
        case State::ERROR_STATE:
            // Action for ERROR_STATE
            break;
    }
}

void handleEvent(Event event) {
    switch (currentState) {
        case State::WAIT:
            if (event == Event::COLOR_NEW_SENSOR_VALUE) {
                currentState = State::COLOR_DETECTION;
            } else if (event == Event::INITIALIZED) {
                // Stay in WAIT or handle initialization
            }
            break;
        case State::COLOR_DETECTION:
            if (event == Event::COLOR_DETECTED) {
                currentState = State::COLOR_SORT;
            } else if (event == Event::COLOR_NOT_DETECTED) {
                currentState = State::COLOR_DUMP;
            }
            break;
        case State::COLOR_SORT:
            if (event == Event::CHUTE_MOVED) {
                currentState = State::CHUTE_MOVE;
            }
            break;
        case State::COLOR_DUMP:
            if (event == Event::CHUTE_MOVED) {
                currentState = State::CHUTE_MOVE;
            }
            break;
        case State::CHUTE_MOVE:
            currentState = State::WAIT; // Assuming CHUTE_MOVED is the only event here
            break;
        case State::ERROR_STATE:
            // Handle error or remain in error state
            break;
    }

    executeActionForState(currentState);
}

void statemachineSetup() {

}

#endif
