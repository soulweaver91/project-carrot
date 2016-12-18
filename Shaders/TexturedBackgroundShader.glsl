uniform sampler2D texture;
uniform vec2 canvasDimensions;
uniform vec4 horizonColor;
uniform vec2 shift;

void main() {
  // Warning! Many magic constants ahead!
  // The current values have been carefully picked after a good amount of experimenting.

  // Float position of pixel to write (between 0 and 1 both directions)
  // y coordinate is reversed since the texture is upside down after display()
  vec2 targetCoord = vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y) / canvasDimensions * 256.0;
  // Distance from top or bottom (1 center of screen, 0 edge of screen)
  float verDistance = 1.3 - abs(2.0 * targetCoord.y - 1.0);
  float horizonDepth = pow(verDistance, 2.0);
  float yShift = targetCoord.y > 0.5 ? 1.0 : 0.0;

  vec2 texturePos =
	vec2(
	  (shift.x / 256.0) + (targetCoord.x - 0.5   ) * (0.5 + (1.5 * horizonDepth)),
	  (shift.y / 256.0) + (targetCoord.y - yShift) * 2.0 * verDistance
	)
  ;
  
  vec4 texColor = texture2D(texture, texturePos);
  float horizonOpacity = clamp(pow(verDistance, 0.8) - 0.2, 0.0, 1.0);

  gl_FragColor = texColor * (1.0 - horizonOpacity) + horizonColor * horizonOpacity;
}