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

#include "MaxSizeWidget.h"

MaxSizeWidget::MaxSizeWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.spinWidth->setButtonSymbols(QAbstractSpinBox::NoButtons);
	ui.spinHeight->setButtonSymbols(QAbstractSpinBox::NoButtons);

	ui.spinWidth->setValue(6000);
	ui.spinHeight->setValue(3000);

	ui.checkEnabled->setChecked(false);
	HandleEnabling();
}

void MaxSizeWidget::HandleEnabling()
{
	bool fChecked = ui.checkEnabled->isChecked();

	ui.label1->setEnabled(fChecked);
	ui.label2->setEnabled(fChecked);
	ui.label3->setEnabled(fChecked);
	ui.label4->setEnabled(fChecked);
	ui.spinWidth->setEnabled(fChecked);
	ui.spinHeight->setEnabled(fChecked);
}

void MaxSizeWidget::OnSpinWidthEditingFinished()
{
	int newWidth = ui.spinWidth->value();
	if (newWidth % 2 != 0)
	{
		newWidth++;
		ui.spinWidth->setValue(newWidth);
	}

	ui.spinHeight->setValue(newWidth / 2);
}

void MaxSizeWidget::OnSpinHeightEditingFinished()
{
	int newHeight = ui.spinHeight->value();
	
	ui.spinWidth->setValue(newHeight * 2);
}

void MaxSizeWidget::OnCheckEnableToggled()
{
	HandleEnabling();
}
