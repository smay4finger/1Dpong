#include <Adafruit_NeoPixel.h>
#include <color.h>

const int playerA_button = 39;
const int playerB_button = 40;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(240, 53, NEO_GRB + NEO_KHZ800);

void setup() {
    randomSeed(analogRead(A0));
    pixels.begin();
    pixels.show();

    pinMode(playerA_button, INPUT_PULLUP);
    pinMode(playerB_button, INPUT_PULLUP);
}

const int16_t SPEED_MIN = 140;
const int16_t SPEED_MAX = 500;
const int16_t PLAYER_AREA = 3000;
const int16_t PLAYER_B = 20000;
const int16_t PLAYER_A = -PLAYER_B;

int16_t ball_position = 0;
int16_t ball_speed = -SPEED_MIN;

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

void draw_tilt() {
    color_t player1_color;
    color_t player2_color;
    uint16_t n;

    if ( ball_speed < 0 ) {
        player1_color = rgb(255, 0, 0);
        player2_color = rgb(0, 255, 0);
    }
    else {
        player1_color = rgb(0, 255, 0);
        player2_color = rgb(255, 0, 0);
    }

    clear();

    for ( n = 0; n < 32; n++ ) {
        pixels.setPixelColor(n, player1_color.packed);
        player1_color = brightness(player1_color, 192);
    }

    for ( n = pixels.numPixels(); n > pixels.numPixels() - 32; n-- ) {
        pixels.setPixelColor(n, player2_color.packed);
        player2_color = brightness(player2_color, 192);
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
    /* KI player */
    if ( ball_speed > 0 && ball_position > PLAYER_B && random(20) > 0) {
        ball_speed = map(random(1000), 0, 1000, -SPEED_MAX, -SPEED_MIN);
        ball_position = PLAYER_B;
    }

    /* Game logic */
    if ( outside_playground() ||
            (pressed(PLAYER_A, playerA_button) && outside_player_area(PLAYER_A)) ||
            (pressed(PLAYER_B, playerB_button) && outside_player_area(PLAYER_B)) ) {
        draw_tilt();
        delay(2000);
        ball_position = 0;
        ball_speed = random(2) == 0 ? -140 : 140;
        return;
    }
    else if ( pressed(PLAYER_A, playerA_button) && inside_player_area(PLAYER_A) ) {
        ball_speed = map(ball_position - PLAYER_A, 0, PLAYER_AREA, SPEED_MAX, SPEED_MIN);
        ball_speed = constrain(ball_speed, SPEED_MIN, SPEED_MAX);
    }
    else if ( pressed(PLAYER_B, playerB_button) && inside_player_area(PLAYER_B) ) {
        ball_speed = map(PLAYER_B - ball_position, 0, PLAYER_AREA, -SPEED_MAX, -SPEED_MIN);
        ball_speed = constrain(ball_speed, -SPEED_MAX, -SPEED_MIN);
    }

    /* calculate new ball position */
    ball_position += ball_speed;


    draw_game();
}
