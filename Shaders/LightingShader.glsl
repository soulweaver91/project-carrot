uniform vec3 color;
uniform float lightingLevel;
uniform vec2 center;
uniform sampler2D texture;

void main() {
  vec2 pos = gl_TexCoord[0].xy;
  vec4 originalColor = texture2D(texture, pos);

  float dist = distance(vec2(gl_FragCoord), center);
  if (dist < 100.0) {
	gl_FragColor = originalColor;
	return;
  }

  if (dist > 200.0) {
    gl_FragColor = vec4(originalColor * lightingLevel);
	return;
  }

  dist -= 100.0;
  float gradientDepth = 1.0 - lightingLevel;
  gl_FragColor = vec4(originalColor * (1.0 - (dist * gradientDepth / 100.0)));
}