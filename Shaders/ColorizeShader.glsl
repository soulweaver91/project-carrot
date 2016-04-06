uniform vec3 color;
uniform sampler2D texture;

void main() {
  vec4 input = texture2D(texture, gl_TexCoord[0].xy);

  // Add to base color by default. Negative values can be used for subtractive dyeing otherwise.
  vec4 dye = vec4(1.0 + color[0], 1.0 + color[1], 1.0 + color[2], 1.0);
  
  // Apply to a grayscale version of the image.
  float average = (input.r + input.g + input.b) / 3.0;
  vec4 gray = vec4(average, average, average, input.a);

  gl_FragColor = vec4(gray * dye);
}