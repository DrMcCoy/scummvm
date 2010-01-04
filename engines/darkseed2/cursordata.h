struct StaticCursor {
	int32 width;
	int32 height;
	int32 hotspotX;
	int32 hotspotY;

	uint32 dataSize;
	byte *data;
};

static byte staticCursorPointerData[] = {
	0x28, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xC0, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00,
	0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00,
	0x03, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00,
	0x66, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00,
	0x7E, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00,
	0x7F, 0x80, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,
	0x7E, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00,
	0x78, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00,
	0x60, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF,
	0xFE, 0x1F, 0xFF, 0xFF, 0xFE, 0x1F, 0xFF, 0xFF,
	0xFC, 0x3F, 0xFF, 0xFF, 0x7C, 0x3F, 0xFF, 0xFF,
	0x38, 0x7F, 0xFF, 0xFF, 0x18, 0x7F, 0xFF, 0xFF,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
	0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x1F, 0xFF, 0xFF,
	0x00, 0x3F, 0xFF, 0xFF, 0x00, 0x7F, 0xFF, 0xFF,
	0x00, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF,
	0x03, 0xFF, 0xFF, 0xFF, 0x07, 0xFF, 0xFF, 0xFF,
	0x0F, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF,
	0x3F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF
};

static StaticCursor staticCursorPointer = {
	32, 32, 0, 0,
	sizeof(staticCursorPointerData),
	staticCursorPointerData
};
