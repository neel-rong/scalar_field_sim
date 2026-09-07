#version 330 core

uniform sampler2D u_FieldTexture;
uniform vec2 u_PixelMinMax;

in vec2 uv;
out vec4 FragColor;

void main()
{
    float value = texture(u_FieldTexture, uv).r;

    float minValue =  u_PixelMinMax.x;
    float maxValue =  u_PixelMinMax.y;

    float range = maxValue - minValue;

    // Avoid division by zero for constant fields.
    float normalizedValue = range > 1e-6
                            ? (value - minValue) / range
                            : 0.025;

    // Constant field: use the minimum visible intensity.
    normalizedValue = clamp(normalizedValue, 0.025, 1.0);

    FragColor = vec4(normalizedValue, normalizedValue, normalizedValue, 1.0);
}