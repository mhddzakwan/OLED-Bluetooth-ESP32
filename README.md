# Wiring
VCC -> 3.3V
GND -> GND
SDA -> D21
SCL -> D22

# Expression
- Marah = ```roboEyes.setMood(ANGRY);```
- Tired = ```roboEyes.setMood(TIRED);```
- Happy = ```roboEyes.setMood(HAPPY);```
- Normal = ```roboEyes.setMood(0);```
- laugh = ```roboEyes.anim_laugh();```
- Confused = ```roboEyes.anim_confused();```
- flicker =
  ```
  static bool flickerState = false;
    flickerState = !flickerState;
    roboEyes.setHFlicker(flickerState ? ON : OFF, 1);
  ```

  # Eyes Size
  - Width = ```roboEyes.setWidth(val, val);```
  - Height = ```roboEyes.setHeight(val, val);```
  - Border Radius = ```roboEyes.setBorderradius(val, val);```
  - Space between Eyes = ```roboEyes.setSpacebetween(val);```
