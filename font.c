#include "font.h"


cairo_font_face_t* Mf_GetFont(const char* font_family) {
    FcInit();

    FcPattern* pattern = FcNameParse((const FcChar8*)font_family);
    FcConfigSubstitute(NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    FcPattern* matched = FcFontMatch(NULL, pattern, &result);

    FcChar8* font_file = NULL;
    int font_index = 0;
    if (matched) {
        FcPatternGetString(matched, FC_FILE, 0, &font_file);
        FcPatternGetInteger(matched, FC_INDEX, 0, &font_index);
    }

    if (!font_file) {
        fprintf(stderr, "Font file not found for '%s'\n", font_family);
        return NULL;
    }

    FT_Library ft_library;
    FT_Face ft_face;

    if (FT_Init_FreeType(&ft_library) != 0 ||
        FT_New_Face(ft_library, (const char*)font_file, font_index, &ft_face) != 0) {
        fprintf(stderr, "Failed to load face from %s\n", font_file);
        return NULL;
    }

    cairo_font_face_t* cairo_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);

    FcPatternDestroy(pattern);
    FcPatternDestroy(matched);
    FcFini();

    return cairo_face;
}

