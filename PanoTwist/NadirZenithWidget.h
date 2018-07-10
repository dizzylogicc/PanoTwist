/* Copyright (c) 2018 Peter Kondratyuk. All Rights Reserved.
*
* You may use, distribute and modify the code in this file under the terms of the MIT Open Source license, however
* if this file is included as part of a larger project, the project as a whole may be distributed under a different
* license.
*
* MIT license:
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
* to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions
* of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#pragma once

#include <QWidget>
#include <QRegExpValidator>
#include "ui_NadirZenithWidget.h"

#include "CvImageWidget.h"
#include "BString.h"

class NadirZenithWidget : public QWidget
{
	Q_OBJECT

public:
	NadirZenithWidget(QWidget* parent = 0);
	~NadirZenithWidget(){}

public slots:
	void OnSettingsChanged() //Interface indicates that the user changed something
	{
		HandleEnabling();
		emit SignalSettingsChanged();
	} 

	void OnSelectImageClicked();
	void OnColorEditingFinished();
	void OnColorTextChanged();

signals:
	void SignalSettingsChanged();

public:
	void SetCheckText(const BString& text) { ui.checkEnable->setText(text.c_str()); }
	void SetColor(const BString& colorString);
	void SetFnadir(bool val) { fNadir = val; }		//Sets whether the widget is responsible for nadir (true) or zenith (false)

	bool IsEnabled() { return ui.checkEnable->isChecked(); }
	int AngleDeg() { return ui.spinAngle->value(); }
	BString ColorString()
	{
		if (curColorString[0] == '#') return curColorString;
		else return BString("#") + curColorString;
	}
	const cv::Mat& PatchImage() { return patchImage; }

	bool IsFillAverage() { return ui.radioAverage->isChecked(); }
	bool IsFillColor() { return ui.radioColor->isChecked(); }
	bool IsFillImage() { return ui.radioImage->isChecked(); }
	bool IsNadir() { return fNadir; }		//Is it nadir or zenith?

	bool IsPatchImageLoaded() { return fPatchImageLoaded; }

private:
	void HandleEnabling();

private:
	QRegExpValidator* colorValidator;
	BString curColorString;

	CvImageWidget* imageWidget;
	cv::Mat patchImage;
	bool fPatchImageLoaded;
	bool fNadir;

private:
	Ui::NadirZenithWidgetClass ui;
};