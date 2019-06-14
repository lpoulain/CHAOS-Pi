# User Interface and Applications

The current system runs one application at a time. No multitasking yet, whether cooperative nor preemptive. Instead, it relies on events.

Each application needs to provide several callback functions:

- An initialization function, called at startup
- The four Touchscreen callbacks: `first_swipe()`, `swipe()`, `first_pinch()` and `pinch()`
- A `process_touch_event()` function, called when a touch event completed, i.e. the user lifts the finger(s) off the screen.

The App Launcher will call the Touchscreen driver's `touchscreen_poll()` function, which will call the application callbacks if need be and return once a touch event completed.

If the touch event is an EXIT event (the user touched the lower left corner of the screen), then the App Launcher displays the main menu. If not, it calls the app's `process_touch_event()` callback function and repeats the procedure.
