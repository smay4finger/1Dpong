#include <Adafruit_NeoPixel.h>
#include <color.h>

const int PLAYER_A_BUTTON = 39;
const int PLAYER_B_BUTTON = 40;
const int PIXEL_PIN = 53;

const int16_t SPEED_MIN = 140;
const int16_t SPEED_MAX = 600;
const int16_t PLAYER_AREA = 3000;
const int16_t PLAYER_B = 20000;
const int16_t PLAYER_A = -PLAYER_B;
const int8_t POINTS_MAX = 3;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(240, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    randomSeed(analogRead(A0));
    pixels.begin();
    pixels.show();

    pinMode(PLAYER_A_BUTTON, INPUT_PULLUP);
    pinMode(PLAYER_B_BUTTON, INPUT_PULLUP);
}

int16_t ball_position = 0;
int16_t ball_speed = 0;
uint8_t playerA_points = 0;
uint8_t playerB_points = 0;

enum state {
    STATE_STOPPED,
    STATE_PLAYER_A_WINS,
    STATE_PLAYER_B_WINS,
    STATE_PLAYER_A_WINS_ROUND,
    STATE_PLAYER_B_WINS_ROUND,
    STATE_GAME,
} state = STATE_STOPPED;

void clear() {
    uint16_t n;
    for ( n = 0; n < pixels.numPixels(); n++ ) {
        pixels.setPixelColor(n, 0, 0, 0);
    }
#if 0
    /* draw helper */
    pixels.setPixelColor(map(PLAYER_A + PLAYER_AREA, PLAYER_A, PLAYER_B, 0, pixels.numPixels()), 255, 255, 255);
    pixels.setPixelColor(map(PLAYER_B - PLAYER_AREA, PLAYER_A, PLAYER_B, 0, pixels.numPixels()), 255, 255, 255);
#endif
}

void draw_stopped() {
    clear();
    pixels.show();
}

void draw_tilt(int16_t player) {
    color_t player1_color;
    color_t player2_color;
    uint16_t n;

    if ( player < 0 ) {
        player1_color = rgb(255, 0, 0);
        player2_color = rgb(0, 255, 0);
    }
    else if ( player > 0 ) {
        player1_color = rgb(0, 255, 0);
        player2_color = rgb(255, 0, 0);
    }
    else {
        player1_color = rgb(255, 255, 255);
        player2_color = rgb(255, 255, 255);
    }

    clear();

    for ( n = 0; n < 32; n++ ) {
        pixels.setPixelColor(n, player1_color.packed);
        player1_color = brightness(player1_color, 192);
    }

    for ( n = 64; n < (uint16_t)(64 + playerB_points * 2); n+=2 ) {
        pixels.setPixelColor(n, 255, 255, 255);
    }

    for ( n = pixels.numPixels(); n > pixels.numPixels() - 32; n-- ) {
        pixels.setPixelColor(n, player2_color.packed);
        player2_color = brightness(player2_color, 192);
    }

    for ( n = pixels.numPixels() - 64; n > pixels.numPixels() - 64 - playerA_points * 2; n-=2 ) {
        pixels.setPixelColor(n, 255, 255, 255);
    }

    pixels.show();
}

void draw_game() {
    uint16_t n;
    color_t color;

    /* clear */
    clear();

    /* paint ball */
    color = rgb(0, 0, 255);

    uint16_t ball_pixel = map(ball_position, PLAYER_A, PLAYER_B, 0, pixels.numPixels());
    pixels.setPixelColor(ball_pixel, color.packed);

    /* paint trail */
    color = brightness(color, 128);
    for ( n = 1; n < 16; n++ ) {
        int position = ball_speed > 0 ? ball_pixel - n : ball_pixel + n;
        color = brightness(color, 192);
        pixels.setPixelColor(position, color.packed);
    }

    /* showtime */
    pixels.show();
}

bool pressed(int16_t player, uint8_t button) {
    return !digitalRead(button) && 
        ((ball_speed < 0 && player < 0) || (ball_speed > 0 && player > 0));
}

bool outside_player_area(int16_t player) {
    return abs(ball_position - player) > PLAYER_AREA;
}

bool inside_player_area(int16_t player) {
    return !outside_player_area(player);
}

bool outside_playground() {
    return (ball_position < min(PLAYER_A, PLAYER_B)) 
        || (ball_position > max(PLAYER_A, PLAYER_B));
}

void loop() {
    /*
     * KI logic
     */
    if ( state == STATE_GAME ) {
        if ( ball_speed > 0 && ball_position > PLAYER_B && random(5) > 0) {
            /* KI Player B */
            ball_speed = map(random(1000), 0, 1000, -SPEED_MAX, -SPEED_MIN);
            ball_position = PLAYER_B;
        }
    }

    if ( state == STATE_PLAYER_B_WINS_ROUND ) {
        ball_speed = -SPEED_MIN;
        ball_position = PLAYER_B;
        state = STATE_GAME;
        delay(1000);
    }

    /*
     * Game logic
     */
    switch ( state ) {
    case STATE_PLAYER_A_WINS:
    case STATE_PLAYER_B_WINS:
        delay(10000);
        state = STATE_STOPPED;
        break;
    case STATE_STOPPED:
        if ( !digitalRead(PLAYER_A_BUTTON) ) {
            ball_speed = SPEED_MIN;
            ball_position = PLAYER_A;
            playerA_points = 0;
            playerB_points = 0;
            state = STATE_GAME;
            delay(100);
        }
        if ( !digitalRead(PLAYER_B_BUTTON) ) {
            ball_speed = -SPEED_MIN;
            ball_position = PLAYER_B;
            playerA_points = 0;
            playerB_points = 0;
            state = STATE_GAME;
            delay(100);
        }
        break;
    case STATE_PLAYER_A_WINS_ROUND:
        if ( !digitalRead(PLAYER_A_BUTTON) ) {
            ball_speed = SPEED_MIN;
            ball_position = PLAYER_A;
            state = STATE_GAME;
            delay(1000);
        }
        break;
    case STATE_PLAYER_B_WINS_ROUND:
        if ( !digitalRead(PLAYER_B_BUTTON) ) {
            ball_speed = -SPEED_MIN;
            ball_position = PLAYER_B;
            state = STATE_GAME;
            delay(1000);
        }
        break;
    case STATE_GAME:
        if ( outside_playground() ||
                (pressed(PLAYER_A, PLAYER_A_BUTTON) && outside_player_area(PLAYER_A)) ||
                (pressed(PLAYER_B, PLAYER_B_BUTTON) && outside_player_area(PLAYER_B)) ) {
            if ( ball_speed < 0 ) {
                playerA_points++;
                if ( playerA_points >= POINTS_MAX ) {
                    state = STATE_PLAYER_A_WINS;
                }
                else {
                    state = STATE_PLAYER_A_WINS_ROUND;
                }
            }
            else {
                playerB_points++;
                if ( playerB_points >= POINTS_MAX ) {
                    state = STATE_PLAYER_B_WINS;
                }
                else {
                    state = STATE_PLAYER_B_WINS_ROUND;
                }
            }
        }
        else if ( pressed(PLAYER_A, PLAYER_A_BUTTON) && inside_player_area(PLAYER_A) ) {
            ball_speed = map(ball_position - PLAYER_A, 0, PLAYER_AREA, SPEED_MAX, SPEED_MIN);
        }
        else if ( pressed(PLAYER_B, PLAYER_B_BUTTON) && inside_player_area(PLAYER_B) ) {
            ball_speed = map(PLAYER_B - ball_position, 0, PLAYER_AREA, -SPEED_MAX, -SPEED_MIN);
        }
        else {
            ball_position += ball_speed;
        }
        break;
    default:
        state = STATE_STOPPED;
    }

    switch ( state ) {
    case STATE_STOPPED:
        draw_stopped();
        break;
    case STATE_PLAYER_A_WINS:
    case STATE_PLAYER_A_WINS_ROUND:
        draw_tilt(PLAYER_A);
        break;
    case STATE_PLAYER_B_WINS:
    case STATE_PLAYER_B_WINS_ROUND:
        draw_tilt(PLAYER_B);
        break;
    case STATE_GAME:
        draw_game();
        break;
    default:
        draw_stopped();
    }
}
