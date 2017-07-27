#pragma once

class MyDialog;
class XMLTag;
class QBoxLayout;
class FEBioModel;
class FEModel;

class UIBuilder
{
public:
	UIBuilder();

	bool BuildUI(MyDialog* dlg, FEBioModel& fem, const char* szfile);

private: // helper functions for parsing app file
	bool parseModel(XMLTag& tag);
	bool parseGUI  (XMLTag& tag);
	bool parseTags     (XMLTag& tag, QBoxLayout* playout);
	void parseGroup    (XMLTag& tag, QBoxLayout* playout);
	void parseVGroup   (XMLTag& tag, QBoxLayout* playout);
	void parseHGroup   (XMLTag& tag, QBoxLayout* playout);
	void parseTabGroup (XMLTag& tag, QBoxLayout* playout);
	void parseInput    (XMLTag& tag, QBoxLayout* playout);
	void parseStretch  (XMLTag& tag, QBoxLayout* playout);
	void parseButton   (XMLTag& tag, QBoxLayout* playout);
	void parseLabel    (XMLTag& tag, QBoxLayout* playout);
	void parseGraph    (XMLTag& tag, QBoxLayout* playout);
	void parsePlot3d   (XMLTag& tag, QBoxLayout* playout);
	void parseInputList(XMLTag& tag, QBoxLayout* playout);

	FEBioModel*	m_fem;			//!< the FE model we're processing
	char		m_szfile[512];	//!< FE model input file name
	MyDialog*	m_dlg;			//!< the dialog we're building
};
