/*
 *  CGuiLabel.h
 *  MobiTracker
 *
 *  Created by Marcin Skoczylas on 10-01-07.
 *  Copyright 2010 Marcin Skoczylas. All rights reserved.
 *
 */

#ifndef _GUI_MULTILINE_LABEL_
#define _GUI_MULTILINE_LABEL_

#include "CGuiElement.h"
#include "CSlrImage.h"
#include "SYS_Main.h"
#include "CSlrString.h"
#include "CSlrFont.h"
#include "CContinuousParam.h"
#include "CPool.h"

#define RICH_TEXT_USE_ELEMENTS_POOL
#define RICH_TEXT_ELEMENTS_POOL	10000

#define RICH_TEXT_LABEL_ALIGNMENT_LEFT		1
#define RICH_TEXT_LABEL_ALIGNMENT_RIGHT		2
#define RICH_TEXT_LABEL_ALIGNMENT_JUSTIFY	3
#define RICH_TEXT_LABEL_ALIGNMENT_CENTER	4

#define RICH_TEXT_LABEL_ELEMENT_TYPE_UNKNOWN		0
#define RICH_TEXT_LABEL_ELEMENT_TYPE_TEXT			1
#define RICH_TEXT_LABEL_ELEMENT_TYPE_IMAGE			2
#define RICH_TEXT_LABEL_ELEMENT_TYPE_LINE_BREAK		3

#define RICH_TEXT_LABEL_ELEMENT_BLINK_MODE_OFF		0
#define RICH_TEXT_LABEL_ELEMENT_BLINK_MODE_LINEAR	1
#define RICH_TEXT_LABEL_ELEMENT_BLINK_MODE_SINUS	2

class CGuiRichTextLabelCallback;

class CGuiRichTextLabelElement
{
public:
	CGuiRichTextLabelElement();
	CGuiRichTextLabelElement(float x, float y, float z, float textWidth, float fontHeight, float spaceWidth, float gapHeight, u8 alignment, u8 blinkMode, float blinkParamMin, float blinkParamMax, u32 blinkParamNumFrames);
	virtual void DoLogic();
	virtual void Render();
	virtual ~CGuiRichTextLabelElement();
	virtual void UpdatePos(float x, float y, float z);

	u8 type;

	float x, y, z;
	float textWidth;
	float spaceWidth;
	float fontHeight;
	float gapHeight;
	u8 alignment;
	
	u8 blinkMode;
	float blinkParamMin;
	float blinkParamMax;
	u32 blinkParamNumFrames;
	CContinuousParam *blinkParam;
	void InitBlink();
	
#ifdef RICH_TEXT_USE_ELEMENTS_POOL
private:
	static CPool poolElements;
public:
	static void* operator new(const size_t size) { return poolElements.New(size); }
	static void operator delete(void* pObject) { poolElements.Delete(pObject); }
#endif
};

class CGuiRichTextLabelElementText : public CGuiRichTextLabelElement
{
public:
	CGuiRichTextLabelElementText(CSlrString *str, CSlrFont *font, float x, float y, float z, float r, float g, float b, float a, float scale,
		 float fontHeight, float spaceWidth, float gapHeight, u8 alignment,
		 u8 blinkMode, float blinkParamMin, float blinkParamMax, u32 blinkParamNumFrames);
	virtual void Render();
	virtual ~CGuiRichTextLabelElementText();
	CSlrString *text;
	CSlrFont *font;
	float r;
	float g;
	float b;
	float a;
	float scale;
	
#ifdef RICH_TEXT_USE_ELEMENTS_POOL
private:
	static CPool poolElementsText;
public:
	static void* operator new(const size_t size) { return poolElementsText.New(size); }
	static void operator delete(void* pObject) { poolElementsText.Delete(pObject); }
#endif
	
};

class CGuiRichTextLabelElementImage : public CGuiRichTextLabelElement
{
public:
	CGuiRichTextLabelElementImage(CSlrImage *image,
			float x, float y, float z, float sizeX, float sizeY, float offsetX, float offsetY,
			float spaceWidth, float gapHeight, u8 alignment,
			u8 blinkMode, float blinkParamMin, float blinkParamMax, u32 blinkParamNumFrames);
	virtual void Render();
	virtual ~CGuiRichTextLabelElementImage();

	CSlrImage *image;
	float offsetX;
	float offsetY;
	virtual void UpdatePos(float x, float y, float z);
	
#ifdef RICH_TEXT_USE_ELEMENTS_POOL
private:
	static CPool poolElementsImage;
public:
	static void* operator new(const size_t size) { return poolElementsImage.New(size); }
	static void operator delete(void* pObject) { poolElementsImage.Delete(pObject); }
#endif
	
};

class CGuiRichTextLabelElementLineBreak : public CGuiRichTextLabelElement
{
public:
	CGuiRichTextLabelElementLineBreak(float fontHeight, float spaceWidth, float gapHeight, u8 alignment,
									  u8 blinkMode, float blinkParamMin, float blinkParamMax, u32 blinkParamNumFrames);
	virtual void Render();
	virtual ~CGuiRichTextLabelElementLineBreak();
	
#ifdef RICH_TEXT_USE_ELEMENTS_POOL
private:
	static CPool poolElementsLineBreak;
public:
	static void* operator new(const size_t size) { return poolElementsLineBreak.New(size); }
	static void operator delete(void* pObject) { poolElementsLineBreak.Delete(pObject); }
#endif
};


class CGuiRichTextLabel : public CGuiElement
{
public:
	CGuiRichTextLabel(char *text, float posX, float posY, float posZ, float sizeX, float sizeY, float scale, CGuiRichTextLabelCallback *callback);
	CGuiRichTextLabel(CSlrString *text, float posX, float posY, float posZ, float sizeX, float sizeY, float scale, CGuiRichTextLabelCallback *callback);
	
	void Init();
	
	virtual ~CGuiRichTextLabel();
	void Render();
	void Render(float posX, float posY);

	bool DoTap(float x, float y);
	bool DoFinishTap(float x, float y);
	bool DoDoubleTap(float x, float y);
	bool DoFinishDoubleTap(float x, float y);
	bool DoMove(float x, float y, float distX, float distY, float diffX, float diffY);
	bool FinishMove(float x, float y, float distX, float distY, float accelerationX, float accelerationY);
	void DoLogic();

	void SetPosition(float posX, float posY, float posZ, float sizeX, float sizeY);
	void SetParameters(CSlrFont *font, float scale, float r, float g, float b, float a, u8 alignment);
	void SetPositionNoParse(float posX, float posY, float posZ, float sizeX, float sizeY);
	void SetParametersNoParse(CSlrFont *font, float scale, float r, float g, float b, float a, u8 alignment);

	bool clickConsumed;
	bool beingClicked;

	// overwrite this
	virtual bool Clicked(float posX, float posY);
	virtual bool Pressed(float posX, float posY);

	void SetText(char *text);
	void SetText(CSlrString *text);
	
	CSlrString *text;

	float scale;
	bool transparentToTaps;

	void Parse();
	std::list<CGuiRichTextLabelElement *> elements;

	CGuiRichTextLabelCallback *callback;

private:
	std::list<u16> tagStopChars;
	std::list<u16> tagOpenStopChars;
	std::list<u16> whiteSpaceChars;

	CSlrFont *currentFont;
	float currentFontScale;

	float startFontColorR;
	float startFontColorG;
	float startFontColorB;
	float startFontColorA;

	float currentFontColorR;
	float currentFontColorG;
	float currentFontColorB;
	float currentFontColorA;

	float currentX;
	float currentY;

	float currentSpaceWidth;
	float currentFontHeight;
	float currentGapHeight;

	u8 currentAlignment;
	
	u8 currentBlinkMode;
	float currentBlinkParamMin;
	float currentBlinkParamMax;
	u32 currentBlinkParamNumFrames;
	
	void UpdateFont();

	void AddTextElements(CSlrString *str);
	void MakeTextLayout();

	float bx1, by1, bx2, by2;

	std::list<CGuiRichTextLabelElement *> textElements;

	std::list<CSlrString *> tags;
	std::list<CSlrString *> vals;

	bool TagExists(const char *tagName);
	CSlrString *GetValueForTag(const char *tagName);
	float GetFloatValueForTag(const char *tagName, float defaultValue);
	int GetIntValueForTag(const char *tagName, int defaultValue);

	void AddElement(CGuiRichTextLabelElement *el);
	
	void DeleteElements();
};

class CGuiRichTextLabelCallback
{
public:
	virtual bool RichTextLabelClicked(CGuiRichTextLabel *label);
	virtual bool RichTextLabelPressed(CGuiRichTextLabel *label);
};



#endif
//_GUI_BUTTON_

