#ifndef SYM_ALPHA_BLENDING
#define SYM_ALPHA_BLENDING

sf::Image AlphaBlending(sf::Image background, sf::Image foreground);

void MakeAlphaBlending(char* background_path, char* foreground_path);

#endif