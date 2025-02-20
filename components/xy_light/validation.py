import esphome.config_validation as cv

def cie_xy(value):
    if isinstance(value, list):
        if len(value) != 2:
            raise cv.Invalid(f"CIE xy must have a length of two, not {len(value)}")
        try:
            x, y = float(value[0]), float(value[1])
        except ValueError:
            # pylint: disable=raise-missing-from
            raise cv.Invalid("CIE xy coordinates must be decimals")

        if x < 0:
            raise cv.Invalid("CIE x must at least be greater the 0.0")
        if x > 0.75:
            raise cv.Invalid("CIE x must at most be less the 0.75")

        if y < 0:
            raise cv.Invalid("CIE y must at least be greater the 0.0")
        if y > 0.85:
            raise cv.Invalid("CIE y must at most be less the 0.85")

        return [x, y]

    raise cv.Invalid(
        "Invalid value '{}' for CIE xy. Only x,y is allowed."
    )


def ct_range(value):
    if isinstance(value, list):
        if len(value) != 2:
            raise cv.Invalid(f"Color temperature range must have a length of two, not {len(value)}")
            
        a, b = cv.color_temperature(value[0]), cv.color_temperature(value[1])

        # save people breaking their brains whats min, and max, especially as mired and kelvin are opposite 
        # the underlying api takes mired, so min = cold, max = warm
        return [min(a, b), max(a,b)]

    raise cv.Invalid(
        "Invalid value '{}' for color temperature range. Only [min,mix] is allowed."
    )

def duv(value):
    if isinstance(value, (int, float)):

        if x < 0:
            raise cv.Invalid("Delta uv must be greater then 0.0")
        if x > 0.8:
            raise cv.Invalid("Delta uv must be less then 0.8")

        return value

    raise cv.Invalid(
        "Invalid value '{}' for Delta uv. Only values between 0.0 and 0.8 is allowed."
    )

_wavelength_unit = cv.float_with_unit("Wavelength", r"(nanometers|nm|)")

def wavelength(value):
    nm = _wavelength_unit(value)
    if nm < 360:
        raise cv.Invalid("Wavelength must at least be greater the 360nm")
    if nm > 830:
        raise cv.Invalid("Wavelength must at most be less the 830nm")

    # convert wavelength to xy value
    return nm