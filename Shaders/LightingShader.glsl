uniform vec4 color;
uniform vec2 center;
uniform float radiusNear;
uniform float radiusFar;
uniform sampler2D texture;

void main() {
  vec2 pos = gl_TexCoord[0].xy;
  vec4 originalColor = texture2D(texture, pos);
  float dist = distance(vec2(gl_FragCoord), center);

  if (dist > radiusFar) {
    gl_FragColor = vec4(originalColor);
	return;
  }

  float strength = clamp(1.0 - ((dist - radiusNear) / (radiusFar - radiusNear)), 0.0, 1.0);
  gl_FragColor = vec4(originalColor.r + color.r * strength,
					  originalColor.g + color.g * strength,
					  originalColor.b + color.b * strength,
					  originalColor.a * (1 - strength) + color.a * strength);
}