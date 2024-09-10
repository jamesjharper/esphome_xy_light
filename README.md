XY Light Component
==================
The ``xy_light`` platform lets you create a single RGB or Color Temperature controlled light,
from any combination of RGB and Warm / White / Cold white lights.

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
        color_temperature_range: [1300K, 20000K]
        default_transition_length: 1s
      - name: "Living Room Cove Light Hue"
        control_type: RGB_SATURATION
        default_transition_length: 1s

    source_color_profile:
      standard: sRGB

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

Usage
-----
The `xy_light` light platform consists of 2 parts. 
 1. list of `controls` that influence the `light` output colour, brightness and white point.
 2. A list of `xy_outputs` which represent the devices which will output the desired light based on their device capabilities.

This platform is unique in that `xy_outputs` are treated as a single device. Ie: all devices work together to achieve the desired colour. This has many benefits such as
- Emulating a correlated colour temperature as low as 1300K and as high as 20,000k
- Leveraging both the high CRI lighting capabilities of CWWW lighting and the flexibility of RGB lighting concurrently
- Improved brightness through shared use of all output devices 
- Highly Accurate colour reproduction through output device profile calibrations

There are however trade-offs which may need to be considered.
- Changes to colour temperature, internally adjust the white point of a device profile. This leads to the novel behaviour of RGB values being influenced by colour temperature, which may not be immediately intuitive. 
- When using RGB + CWWW entities,  Home Assistant can have some undesirable interlock behaviours when switching between adjusting colour temperature and RGB. This can be overcome by configuring separate 'controls'. This will result in individual entities for both colour temperature and RGB which may not be desirable. 
- Depending on your hardware characteristics, additional steps may be required to calibrate the light colour.  Hardware requiring calibration may exhibit purple or green hues which may very at different brightnesses. 


`xy_light` Configuration
------------------------
- **controls** (**Required**, list): One or more `XyLightControl`(s) that control this this output device. 
- **xy_outputs** (**Required**, list): a list of `XyOutput` that can convert xy color space values, into the equivalent color produced by hardware. This list can take any combination of the following:
  - ``rgb`` - A device capable of outputting trichromatic values 
  - ``rgb_cwww`` - A device capable outputting trichromatic values + Warm and cold white Correlated colour temperature value 
  - ``rgbw`` - A device capable outputting trichromatic values + white Correlated colour temperature value
  - ``cwww`` - A device capable of outputting Warm and cold white Correlated colour temperature values 
  - ``white`` - A device capable of outputting white Correlated colour temperature value
  - ``id`` - a reference to a `XyOutput` defined elsewhere within the program
- **source_color_profile** (*Optional*, `RgbProfile`): At this time ESPHome does not support receiving XY values from Home Assistant. This profile is used to convert the input RGB values into the xy colour space. 
*The default is set to sRGB which should work most if not all HA companion apps and browsers*
- **calibration_logging** (*Optional*, `bool`): When enabled, XY and XYZ values are logged and colour temperature is fixed to the source profiles white point



`XyLightControl` Configuration
-------------------------------
- **name** (**Required**, string): The name of the light.
- **color_temperature_range** (*Optional*, `[{warm}, {cold}]`): Specifies the colour temperature range available in HA. Note, this can be __any__ value, so long as a combination of output devices can output the desired colour. Most RGB values devices are capable of emulating a correlated colour temperature as low as 1000K and as high as 20,000k. Native Correlated Colour Temperature devices will be used within their capable ranges. *Note: Internally, Colour temperature values are calculated using McCamy's approximation. This produces accurate values between ~ 2000K - ~15,000k and *useable* values from ~1300K - ~20,000k. Outside of these values, you may experience flickering and other inaccuracies.
- **control_type** (**Required**, `enum`): Sets what capabilities are available on the HA entity.
  - ``RGB`` - Entity can control the RGB value, brightness, off/on state
  - ``RGB_SATURATION`` - Entity can control the RGB value, colour saturation, and hue off/on state (typically used in conjunction with another `XyLightControl` set as `CT`)
  - ``CT`` - Entity can control the Colour Temperature, brightness, and off/on the state of the xy output devices
  - ``RGB_CT`` Entity can control the RGB value, Colour Temperature, brightness, off/on state *Note: this control has a usual interlock behavior which may make it unsuitable for your use*
  - ``RGB_CWWW`` - Entity can control the RGB value, warm white/cold white, brightness, and off/on state. *Note: as warm white and cold white values are emulated across all devices, this does not behave as expected*
- All other options from :ref:`Light <config-light>`.


`XyOutput`: rgb Configuration
-------------------------------
- **red** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the red channel.
- **green** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the green channel.
- **blue** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the blue channel.
- **calibration_logging** (**Optional**, `bool`): When enabled, normalized RGB values are logged which can be used for calibrating the intensity values against a known source value
- **rgb_profile** (**Required**, `RgbProfile`): The CIE RGB profile used to transform xy values to the output channel intensities. See `RgbProfile` section

`XyOutput`: cwww Configuration
-------------------------------
- **warm_white** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the warm white channel.
- **cold_white** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the cold white channel.
- **calibration_logging** (**Optional**, `bool`): When enabled, warm/cold white intensity values are logged which can be used for calibrations.
- **cwww_profile** (**Required**, `CwwwProfile`): The CIE CWWW profile used to transform xy values to the output channel intensities. See `CwwwProfile` section

`XyOutput`: w Configuration
-------------------------------
- **white** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the warm white channel.
- **cold_white** (*Optional*, :ref:`config-id`): The id of the float :ref:`output` to use for the cold white channel.
- **calibration_logging** (**Optional**, `bool`): When enabled, white intensity values are logged which can be used for calibrations.
- **white_profile** (**Required**, `whiteProfile`): The CIE white profile used to transform xy values to the output channel intensities. See `WhiteProfile` section


`RgbProfile` Configuration
-------------------------------
- **standard** (*Optional*, `enum`): Use pre-configured profile values.
  - ``led`` - Profile with chromaticities of a typical Red, Green and Blue LED. Use this as a starting point, and calibrate as necessary.
  - ``sRGB`` - A standard profile used by most consumer hardware.
  - ``ACES AP0`` - Profile which contains all possible colours in the xy colour space, including imaginary colours. *Useful for calibrating*
  - ``ACES AP1`` - Profile which contains all practical colours in the xy colour space.v*Useful for calibrating*
 - **{red/green/blue}_xy** (*Optional*, `[x,y]`): Chromaticity of the output, ie the real-world colour that is produced by the device *Note: calibrating these values will be necessary if red/green/blue hues do not adequately correlate with the value shown in HA. This can be done using color-accurate display and your eyes, knowledge of the chromaticity or wavelength values from the manufacturer or using a Spectrometer*
 - **{red/green/blue}_wavelength** (*Optional*, `nm`): Domint wavelength of the device. You may find this on the device spec sheet. At build time this is converted into a xy value.
 - **{red/green/blue}_intensity** (*Optional*, `float`): The amount of attenuation required for each colour to render the white point at the maximum brightness. ie, when red, green and blue are each set 100%, by how much must two colours be reduced to have the resulting colour as appearing perfectly white (ie white point)? *Note: calibrating these values will be necessary if you see a red, blue, green or purple cast when outputting a neutral white colour. This can be done using reference sources and your eyes or buying a Spectrometer. Typically green will need attenuating on unattenuated RGB leds*
- **white_point** (*Optional*, `K/mired`): White point colour temperature. This is what the "observer" considered as "white". Defaults to D65
- **white_point_xy** (*Optional*, `[x,y]`): Chromaticity of the profile's white point. 
- **gamma** (*Optional*, `flat`): Mostly an aesthetical choice as gamma is already decompressed into the xy space. *Default is to apply no gamma adjustment*


`CwwwProfile` Configuration
-------------------------------

- **id** (*Optional*, :ref:`config-id`): Manually specify the ID used for code generation.
- **cold_white** (**Required**, `K/mired`): Colour temperature of the cold white device
- **warm_white** (**Required**, `K/mired`): Colour temperature of the warm white device
- **white_point** (*Optional*, `K/mired`): White point colour temperature. This is what the "observer" considered as "white". Default is the halfway point between warm and white.
- **red_cct_impurity** (*Optional*, `mired`): The allowable distance from the Planckian locus point corresponding with the warm white device. Too smaller the value, the more noticeable the transition from warm to red light will be, too large will cause reds to appear orange. Adjust this if Reds are becoming too orange or if you notice flickering when changing between red hues (Caused by inaccuracies in McCamy's approximation at low temperatures, you may also consider increasing the color_temperature_range in the `XyControl`, values below 1500K can have issues). *Recommend to only express this value in mired not kelvin, default value is 100 mired.*
- **blue_cct_impurity** (*Optional*, `mired`): The allowable distance from the Planckian locus point corresponding with the cold white device. Too smaller the value, the more noticeable the transition from cold to blue light will be, too large will wash out blue colours. Wash-out is mostly imperceptible, so adjust this if you want to artificially extend the brightness of light blue colours. (Especially useful when using extreme colour temperatures (10K+) to simulate the blue sky (Rayleigh scattering) for ambient daytime lighting). *Recommend to only express this value in mired not kelvin, default value is 10 mired.*
- **{green/purple}_tint_impurity** (*Optional*, `delta UV`): The allowable distance from the Planckian locus in the green/purple direction. Too smaller the value, the more noticeable the transition from white to green/purple light will be, too larger the more washed-out greens/purple will appear. *Only adjust this value if needed, otherwise leave as the default value of 0.06(green) and 0.05(purple)*
- **impurity_gamma_decay** (*Optional*, `float`): The rate of attenuation as the target xy value deviates from the ideal Planckian locus interval. *Increase this value to reduce colour washout, default value is 1.5 mired.*
- **gamma** (*Optional*, `flat`): Mostly an aesthetical choice as gamma is already decompressed into the xy space. Can be used to reduce the effect of white LEDs which become inaccurate at very low intensities with positive curvature (ie a gamma value below 1.0). *Default is to apply no gamma adjustment*


`WhiteProfile` Configuration
-------------------------------
- **id** (*Optional*, :ref:`config-id`): Manually specify the ID used for code generation.
- **white_point** (**Required**, `K/mired`): Colour temperature of the warm white device
- **red_cct_impurity** (*Optional*, `mired`): The allowable distance from the Planckian locus point corresponding with the white point. Adjust this if oranges are becoming too washed. *Recommend to only express this value in mired not kelvin, default value is 100 mired.*
- **blue_cct_impurity** (*Optional*, `mired`): The allowable distance from the Planckian locus point corresponding with the white point. Too smaller the value, the more noticeable the transition from cold to blue light will be, too large will wash out blue colours. Wash-out is mostly imperceptible, so adjust this if you want to artificially extend the brightness of light blue colours. Especially useful when using extreme colour temperatures (10K+) to simulate the blue sky (Rayleigh scattering) for ambient daytime lighting. *Recommend to only express this value in mired not kelvin, default value is 10 mired.*
- **{green/purple}_tint_impurity** (*Optional*, `delta UV`): The allowable distance from the Planckian locus in the green/purple direction. Too small a value, the more noticeable the transition to white light will be, too large the more washed-out greens/purple will appear. *Only adjust this value if needed, otherwise leave as the default value of 0.06(green) and 0.05(purple)*
- **impurity_gamma_decay** (*Optional*, `float`): The rate of attenuation as the target xy value deviates from the idea of Planckian locus interval. *Increase this value to reduce colour washout, default value is 1.5 mired.*
- **gamma** (*Optional*, `flat`): Mostly an aesthetical choice as gamma is already decompressed into the xy space. Can be used to reduce the effect of white LEDs which become inaccurate at very low intensities with positive curvature (ie a gamma value below 1.0). *Default is to apply no gamma adjustment*

 








Thank you Linda for entertaining my comment. I think I better understand now, accentually we are asking how we might make Named users offer some subset of the Tflex benefits without needing to transition them to the full-fat Tflex solution.

1: Probably not relevant based on the content, but on this topic, i belive MUS is the only way customer have access to 
