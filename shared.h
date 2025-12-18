float linearize(float x)
{
   if (x <= 0.04045f)
      return x / 12.92f;
   else
      return pow((x + 0.055f) / 1.055f, 2.4f);
}

vec4 rgba(float r, float g, float b, float a) {
   return {linearize(r / 255), linearize(g / 255), linearize(b / 255), a};
}