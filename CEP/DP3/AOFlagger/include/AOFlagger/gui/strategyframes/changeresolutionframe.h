/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CHANGERESOLUTIONFRAME_H
#define CHANGERESOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../rfi/strategy/changeresolutionaction.h"

#include "../editstrategywindow.h"

class ChangeResolutionFrame : public Gtk::Frame {
	public:
		ChangeResolutionFrame(rfiStrategy::ChangeResolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Change resolution"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_timeDecreaseFactorLabel("Time decrease factor:"),
		_timeDecreaseFactorScale(0, 128, 1),
		_frequencyDecreaseFactorLabel("Frequency decrease factor:"),
		_frequencyDecreaseFactorScale(0, 256, 1),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_timeDecreaseFactorLabel);
			_timeDecreaseFactorLabel.show();

			_box.pack_start(_timeDecreaseFactorScale);
			_timeDecreaseFactorScale.set_value(_action.TimeDecreaseFactor());
			_timeDecreaseFactorScale.show();

			_box.pack_start(_frequencyDecreaseFactorLabel);
			_frequencyDecreaseFactorLabel.show();

			_box.pack_start(_frequencyDecreaseFactorScale);
			_frequencyDecreaseFactorScale.set_value(_action.FrequencyDecreaseFactor());
			_frequencyDecreaseFactorScale.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ChangeResolutionFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ChangeResolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _timeDecreaseFactorLabel;
		Gtk::HScale _timeDecreaseFactorScale;
		Gtk::Label _frequencyDecreaseFactorLabel;
		Gtk::HScale _frequencyDecreaseFactorScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetTimeDecreaseFactor((size_t) _timeDecreaseFactorScale.get_value());
			_action.SetFrequencyDecreaseFactor((size_t) _frequencyDecreaseFactorScale.get_value());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // CHANGERESOLUTIONFRAME_H
