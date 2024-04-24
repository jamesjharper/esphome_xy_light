# XY lights for esphome
3 Color LEDs? Why not 5 Color LEDs?

## Sample Config 
``` yaml
external_components:
  - source:
      type: git
      url: https://github.com/jamesjharper/esphome_xy_light

light:
  - platform: xy_light
    controls:
      - name: "Living Room Cove Light"
        control_type: CWWW
        color_temperature_range: [1000K, 20000K]
        default_transition_length: 1s
      - name: "Living Room Cove Light Hue"
        control_type: RGB_SATURATION
        default_transition_length: 1s

    source_color_profile:
      standard: led
    xy_outputs:
      - rgb_cwww:
          red: red_channel
          green: green_channel
          blue: blue_channel
          cold_white: warm_white_channel
          warm_white: cold_white_channel
          cwww_profile: 
            warm_white: "1800 K"
            cold_white: "6500 K"
            gamma: 1.8
          rgb_profile:
            standard: led
            red_intensity: 100%
            green_intensity: 65% 
            blue_intensity: 58%

output:
  - id: red_channel
    platform: ledc
    pin: GPIO32

  - id: green_channel
    platform: ledc
    pin: GPIO33

  - id: blue_channel
    platform: ledc
    pin: GPIO13

  - id: warm_white_channel
    platform: ledc
    pin: GPIO04

  - id: cold_white_channel
    platform: ledc
    pin: GPIO16
```



