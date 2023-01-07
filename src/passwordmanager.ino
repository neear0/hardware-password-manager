#include <Keyboard.h>
#include <KeyboardLayout.h>
#include <Keyboard_de_DE.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define BTN_PREV_PIN 9
#define BTN_NXT_PIN 5
#define BTN_OUT_PIN 6
#define WELCOME_TXT "Welcome, name!"
#define DEBUG_PRINT 0
#define CURRENT_LAYOUT KeyboardLayout_de_DE

// For GER Layout: character ^ and ` is not supported!
// Check your layout here: https://github.com/arduino-libraries/Keyboard/tree/master/src

constexpr char* passwords[] = {
  "8TsA46gT9P@KNrG9",
  "1234567890`-=[]\\;',./~!@#$%^&*()_+{}|:\"<>?qwertyuiopasdfghjklzxcvbnm",
  "asd"
};

constexpr char* alias[] = {
  "test password",
  "char test",
  "asd"
};

void DebugPrint(String message) {
#if DEBUG_PRINT
  if (Serial) {
    Serial.println(message.c_str());
  }
#endif
}

void DisplayPage(String tabname, bool showLeft = true, bool showRight = true) {
  lcd.setCursor(0, 1);
  if (showLeft)
    lcd.print(" < ");

  int tabNameLength = strlen(tabname.c_str());

  if (tabNameLength > 11)
    tabNameLength = 10;

  int center = 8 - tabNameLength / 2;

  lcd.setCursor(center, 1);
  lcd.print(tabname);

  lcd.setCursor(13, 1);
  if (showRight)
    lcd.print(" > ");
}

void PrintLCDCentered(const char* msg, int line = 0) {
  int messageLength = strlen(msg);

  if (messageLength > 17)
    messageLength = 16;

  int center = 8 - messageLength / 2;

  lcd.setCursor(center, line);
  lcd.print(msg);
}

void runTest();

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  PrintLCDCentered("Initializing..");

  Keyboard.begin(CURRENT_LAYOUT);  // Become a Keyboard with German Keyboard Layout
  delay(1500);

  pinMode(BTN_PREV_PIN, INPUT_PULLUP);
  pinMode(BTN_NXT_PIN, INPUT_PULLUP);
  pinMode(BTN_OUT_PIN, INPUT_PULLUP);

  delay(500);

  PrintLCDCentered(WELCOME_TXT);
  DisplayPage("Home");
}

void drawPage(int currentPage) {
  lcd.setCursor(0, 0);
  static int sizeOfPasswords = sizeof(passwords) / sizeof(passwords[0]);

  if (currentPage == 0) {
    PrintLCDCentered(WELCOME_TXT);
    DisplayPage("Home", currentPage != 0);
    return;
  }

  //handle error pages
  switch (currentPage) {
    case -1:
      PrintLCDCentered("Malformed Data!");
      PrintLCDCentered("size: alias!=pw", 1);
      return;
      break;
    default:
      break;
  }

  int selectedPassword = currentPage - 1;

  PrintLCDCentered(alias[selectedPassword]);
  DisplayPage("Page: " + String(currentPage), currentPage != 0, currentPage < sizeOfPasswords);
}

// DO NOT REPLACE WITH Keyboard.print() or .println() or .write or any of that !!!
// THE MENTIONED FUNCTIONS DO NOT WORK, THE DO CUM ENTATION IS LYING!!!
// Pasted from https://github.com/arduino-libraries/Keyboard/blob/25ab3cdfd2d2cf93f2315fd4c13ef22c9622ba50/src/Keyboard.cpp#L190

void KeyboardOut(String message) {
  Keyboard.releaseAll();
  for (int i = 0; i < message.length(); i++) {
    char current = message.charAt(i);
    Keyboard.press(current);  // Keydown
    delay(150);
    Keyboard.release(current);  // Keyup
  }
}

void DoPasswordOut(int currentPage) {
  if (currentPage <= 0){
    lcd.clear();
    PrintLCDCentered("Error, no pswrd");
    PrintLCDCentered("select with < >", 1);
    return;
  }

  static int sizeOfPasswords = sizeof(passwords) / sizeof(passwords[0]);
  if (currentPage - 1 > sizeOfPasswords)
    return;

  KeyboardOut(passwords[currentPage - 1]);  // Output the stored password
}

bool CheckButton(int pin) {
  return digitalRead(pin) == LOW;
}

void loop() {
  static int currentPage = 0;
  static int lastPage = 0;
  static int sizeOfAliases = sizeof(alias) / sizeof(alias[0]);
  static int sizeOfPasswords = sizeof(passwords) / sizeof(passwords[0]);

  if (sizeOfPasswords != sizeOfAliases) {
    currentPage = -1;
  }

  if (currentPage != lastPage) {
    //we changed pages, clear page to avoid artifacts
    DebugPrint("changed pages, clearing lcd");
    lcd.clear();
    lastPage = currentPage;
  }

  drawPage(currentPage);

  if (CheckButton(BTN_NXT_PIN)) {
    currentPage++;
    DebugPrint("page + 1");
    delay(250);
  } else if (CheckButton(BTN_PREV_PIN)) {
    DebugPrint("page - 1");
    currentPage--;
    delay(250);
  }

  if (currentPage > sizeOfPasswords) {
    currentPage = 1;
    DebugPrint("page = 1");
  } else if (currentPage < 0) {
    currentPage = sizeOfPasswords;
    DebugPrint("page clamp to max size");
  }


  if (CheckButton(BTN_OUT_PIN)) {
    lcd.clear();
    PrintLCDCentered("Printing..");
    DoPasswordOut(currentPage);
    lcd.clear();
  }
}
