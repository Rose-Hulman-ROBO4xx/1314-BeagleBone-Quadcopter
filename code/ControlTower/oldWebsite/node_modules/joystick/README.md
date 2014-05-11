# node-joystick

A node module for reading joystick data based on the work of [Nodebits](http://nodebits.org/linux-joystick).

## Example

```javascript
// Set a deadzone of +/-3500 (out of +/-32k) and a sensitivty of 350 to reduce signal noise in joystick axis
var joystick = new (require('joystick'))(0, 3500, 350);
joystick.on('button', console.log);
joystick.on('axis', console.log);
```