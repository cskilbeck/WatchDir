//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Exec
{
	tstring mCommand;
	bool mAsync;

	Exec(xml_node<> *exec);
	tstring ReplaceAllTokens(tstring const &command, FileEvent *event);
	void Execute(tstring const &folder, FileEvent *fileEvent, vector<HANDLE> &handles);
};

//////////////////////////////////////////////////////////////////////

struct Command
{
	struct ThreadParams
	{
		Command *mCommand;
		tstring const &mFolder;
		FileEvent *mFileEvent;

		ThreadParams(Command *command, tstring const &folder, FileEvent *fileEvent);

		DWORD Run();
	};

	bool mAsync;
	DWORD mFilter;
	vector<Exec> mExecs;

	//////////////////////////////////////////////////////////////////////

	Command(xml_node<> *commandNode);

	//////////////////////////////////////////////////////////////////////

	static DWORD WINAPI ExecuteThread(LPVOID param);

	//////////////////////////////////////////////////////////////////////

	void Execute(tstring const &folder, FileEvent *fileEvent);
};
