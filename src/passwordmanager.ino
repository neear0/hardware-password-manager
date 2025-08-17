#include <KeyboardUTF8.h>
#include <languages/Keyboard_CZ.h>     
#include <languages/Keyboard_DE.h>  
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Button pins
#define btn_prev_pin 9
#define btn_nxt_pin 5
#define btn_out_pin 6

#define welcome_txt "Welcome, name!"
#define debug_print_enabled 0

// Layouts
#define layout_cz Keyboard_CZ
#define layout_de Keyboard_DE

constexpr char* passwords[] = {
  "8TsA46gT9P@KNrG9",
  "1234567890`-=[]\\;',./~!@#$%^&*()_+{}|:\"<>?qwertyuiopasdfghjklzxcvbnm",
  "asd"
};

constexpr char* aliases[] = {
  "test password",
  "char test",
  "asd"
};

void debug_print(const String& message) {
#if debug_print_enabled
  if (Serial) {
    Serial.println(message);
  }
#endif
}

void display_page(const String& tabname, bool show_left = true, bool show_right = true) {
  lcd.setCursor(0, 1);
  if (show_left) lcd.print(" < ");

  int len = tabname.length();
  if (len > 11) len = 10;

  int center = 8 - len / 2;
  lcd.setCursor(center, 1);
  lcd.print(tabname);

  lcd.setCursor(13, 1);
  if (show_right) lcd.print(" > ");
}

void print_lcd_centered(const char* msg, int line = 0) {
  int len = strlen(msg);
  if (len > 17) len = 16;
  int center = 8 - len / 2;

  lcd.setCursor(center, line);
  lcd.print(msg);
}

void keyboard_out(const String& message) {
  KeyboardUTF8.releaseAll();
  for (char c : message) {
    KeyboardUTF8.press(c);
    delay(150);
    KeyboardUTF8.release(c);
  }
}

void do_password_out(int current_page) {
  if (current_page <= 0) {
    lcd.clear();
    print_lcd_centered("Error, no pswrd", 0);
    print_lcd_centered("select with < >", 1);
    return;
  }

  int count = sizeof(passwords) / sizeof(passwords[0]);
  if (current_page - 1 >= count) return;

  keyboard_out(passwords[current_page - 1]);
}

bool check_button_debounced(int pin) {
  static unsigned long last_press[3] = {0,0,0};
  const unsigned long debounce_time = 200;

  int idx = (pin == btn_prev_pin) ? 0 : (pin == btn_nxt_pin) ? 1 : 2;
  if (digitalRead(pin) == LOW) {
    if (millis() - last_press[idx] > debounce_time) {
      last_press[idx] = millis();
      return true;
    }
  }
  return false;
}

void draw_page(int current_page) {
  lcd.setCursor(0, 0);
  int count = sizeof(passwords) / sizeof(passwords[0]);

  if (current_page == 0) {
    print_lcd_centered(welcome_txt, 0);
    display_page("Home", false, count > 0);
    return;
  }
  if (current_page == -1) {
    print_lcd_centered("Malformed Data!");
    print_lcd_centered("size: alias!=pw", 1);
    return;
  }

  int sel = current_page - 1;
  print_lcd_centered(aliases[sel], 0);
  display_page("Page: " + String(current_page), current_page != 0, current_page < count);
}

void setup() {
  lcd.init();
  lcd.backlight();
  print_lcd_centered("Initializing..", 0);

  KeyboardUTF8.begin(layout_cz); 
  delay(1500);

  pinMode(btn_prev_pin, INPUT_PULLUP);
  pinMode(btn_nxt_pin, INPUT_PULLUP);
  pinMode(btn_out_pin, INPUT_PULLUP);

  delay(500);

  print_lcd_centered(welcome_txt, 0);
  display_page("Home");
}

void loop() {
  static int current_page = 0, last_page = -99;
  int pw_count = sizeof(passwords) / sizeof(passwords[0]);
  int alias_count = sizeof(aliases) / sizeof(aliases[0]);

  if (pw_count != alias_count) current_page = -1;

  if (current_page != last_page) {
    debug_print("changed pages, clearing lcd");
    lcd.clear();
    last_page = current_page;
  }

  draw_page(current_page);

  if (check_button_debounced(btn_nxt_pin)) {
    current_page++;
    debug_print("page + 1");
  } else if (check_button_debounced(btn_prev_pin)) {
    current_page--;
    debug_print("page - 1");
  }

  if (current_page > pw_count)
    current_page = 1;
  else if (current_page < 1)
    current_page = pw_count;

  if (check_button_debounced(btn_out_pin)) {
    lcd.clear();
    print_lcd_centered("Printing..");
    do_password_out(current_page);
    delay(1000);
    lcd.clear();
    draw_page(current_page);
  }
}
