; Inno Setup Script for Traintastic

#define VersionInfoBinary "..\..\server\build\traintastic-server.exe"

#define Name "Traintastic"
#define Version GetFileVersion(VersionInfoBinary)
#define VersionFull GetStringFileInfo(VersionInfoBinary, FILE_VERSION)
#define Copyright GetFileCopyright(VersionInfoBinary)
#define Publisher "Reinder Feenstra"
#define URL "https://traintastic.org"
#define ServerExeName "traintastic-server.exe"
#define ClientExeName "traintastic-client.exe"
#define SimulatorExeName "traintastic-simulator.exe"

#define CompanySubKey "SOFTWARE\traintastic.org"
#define AppSubKey CompanySubKey + "\Traintastic"

[Setup]
AppId={{7E509202-257F-4859-B8FA-D87D636342BB}
AppName={#Name}
AppVersion={#Version}
AppVerName={#Name} v{#VersionFull}
AppCopyright={#Copyright}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
VersionInfoVersion={#Version}
VersionInfoProductTextVersion={#VersionFull}
DefaultDirName={autopf}\{#Name}
DisableDirPage=yes
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
OutputDir=output
OutputBaseFilename=traintastic-setup-v{#VersionFull}
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
MinVersion=10.0

[Languages]
Name: en; MessagesFile: "compiler:Default.isl,en-us.isl"
Name: nl; MessagesFile: "compiler:Languages\Dutch.isl,nl-nl.isl"
Name: de; MessagesFile: "compiler:Languages\German.isl,de-de.isl"
Name: it; MessagesFile: "compiler:Languages\Italian.isl,it-it.isl"
Name: sv; MessagesFile: "Languages\Swedish.isl,sv-se.isl"
Name: fr; MessagesFile: "compiler:Languages\French.isl,fr-fr.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "firewall_traintastic"; Description: "{cm:firewall_allow_traintastic_client}"; GroupDescription: "{cm:windows_firewall}"; Check: InstallServer
Name: "firewall_wlanmaus"; Description: "{cm:firewall_allow_wlanmaus_z21}"; GroupDescription: "{cm:windows_firewall}"; Check: InstallServer
Name: "firewall_traintastic_simulator"; Description: "{cm:firewall_allow_traintastic_simulator}"; GroupDescription: "{cm:windows_firewall}"; Check: InstallSimulator

[Files]
; Server
Source: "..\..\server\build\{#ServerExeName}"; DestDir: "{app}\server"; Flags: ignoreversion; Check: InstallServer
; Client
Source: "..\..\client\build\Release\{#ClientExeName}"; DestDir: "{app}\client"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\*.dll"; DestDir: "{app}\client"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\generic\*.dll"; DestDir: "{app}\client\generic"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\iconengines\*.dll"; DestDir: "{app}\client\iconengines"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\imageformats\*.dll"; DestDir: "{app}\client\imageformats"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\networkinformation\*.dll"; DestDir: "{app}\client\networkinformation"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\platforms\*.dll"; DestDir: "{app}\client\platforms"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\styles\*.dll"; DestDir: "{app}\client\styles"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\client\build\Release\tls\*.dll"; DestDir: "{app}\client\tls"; Flags: ignoreversion; Check: InstallClient
; Simulator
Source: "..\..\simulator\build\Release\{#SimulatorExeName}"; DestDir: "{app}\simulator"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\*.dll"; DestDir: "{app}\simulator"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\generic\*.dll"; DestDir: "{app}\simulator\generic"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\iconengines\*.dll"; DestDir: "{app}\simulator\iconengines"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\imageformats\*.dll"; DestDir: "{app}\simulator\imageformats"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\networkinformation\*.dll"; DestDir: "{app}\simulator\networkinformation"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\platforms\*.dll"; DestDir: "{app}\simulator\platforms"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\styles\*.dll"; DestDir: "{app}\simulator\styles"; Flags: ignoreversion; Check: InstallSimulator
Source: "..\..\simulator\build\Release\tls\*.dll"; DestDir: "{app}\simulator\tls"; Flags: ignoreversion; Check: InstallSimulator
; Shared
Source: "..\..\shared\translations\*.lang"; DestDir: "{commonappdata}\traintastic\translations"; Flags: ignoreversion;
; Manual
Source: "..\..\manual\build\*"; DestDir: "{commonappdata}\traintastic\manual"; Flags: ignoreversion recursesubdirs
Source: "..\..\manual\build.luadoc\*"; DestDir: "{commonappdata}\traintastic\manual-lua"; Flags: ignoreversion recursesubdirs
; LNCV XML
Source: "..\..\shared\data\lncv\xml\*.xml"; DestDir: "{commonappdata}\traintastic\lncv"; Flags: ignoreversion; Check: InstallClient
Source: "..\..\shared\data\lncv\xml\lncvmodule.xsd"; DestDir: "{commonappdata}\traintastic\lncv"; Flags: ignoreversion; Check: InstallClient
; Simulator layouts
Source: "..\..\simulator\layout\*.json"; DestDir: "{commonappdata}\traintastic\layout"; Flags: ignoreversion; Check: InstallSimulator
; VC++ redistributable runtime. Extracted by VC2019RedistNeedsInstall(), if needed.
Source: "..\..\client\build\Release\vc_redist.x64.exe"; DestDir: {tmp}; Flags: dontcopy

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; StatusMsg: "Installing VC++ redistributables..."; Parameters: "/quiet /norestart"; Check: VC2019RedistNeedsInstall; Flags: waituntilterminated
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic server (TCP)"" dir=in program=""{app}\server\{#ServerExeName}"" protocol=TCP localport=5740 action=allow"; StatusMsg: "{cm:add_firewall_rule_traintastic_client} (TCP)"; Flags: runhidden; Check: InstallServer; Tasks: firewall_traintastic
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic server (UDP)"" dir=in program=""{app}\server\{#ServerExeName}"" protocol=UDP localport=5740 action=allow"; StatusMsg: "{cm:add_firewall_rule_traintastic_client} (UDP)"; Flags: runhidden; Check: InstallServer; Tasks: firewall_traintastic
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic server (WLANmaus/Z21)"" dir=in program=""{app}\server\{#ServerExeName}"" protocol=UDP localport=21105 action=allow"; StatusMsg: "{cm:add_firewall_rule_wlanmaus_z21}"; Flags: runhidden; Check: InstallServer; Tasks: firewall_wlanmaus
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic simulator (TCP)"" dir=in program=""{app}\simulator\{#SimulatorExeName}"" protocol=TCP localport=5741 action=allow"; StatusMsg: "{cm:add_firewall_rule_traintastic_simualtor} (TCP)"; Flags: runhidden; Check: InstallSimulator; Tasks: firewall_traintastic_simulator

[InstallDelete]
; Delete old translation files (TODO: remove in 0.4)
Type: files; Name: "{commonappdata}\traintastic\translations\en-us.txt"
Type: files; Name: "{commonappdata}\traintastic\translations\nl-nl.txt"
Type: files; Name: "{commonappdata}\traintastic\translations\de-de.txt"
Type: files; Name: "{commonappdata}\traintastic\translations\it-it.txt"
; Delete unused DLLs, now statically linked (TODO: remove in 0.4)
Type: files; Name: "{app}\server\lua53.dll"
Type: files; Name: "{app}\server\lua54.dll"
Type: files; Name: "{app}\server\archive.dll"
Type: files; Name: "{app}\server\zlib1.dll"
; Delete Qt5 DLLs (TODO: remove in 0.4)
Type: files; Name: "{app}\client\libEGL.dll"
Type: files; Name: "{app}\client\libGLESv2.dll"
Type: files; Name: "{app}\client\Qt5Xml.dll"
Type: files; Name: "{app}\client\Qt5Core.dll"
Type: files; Name: "{app}\client\Qt5Gui.dll"
Type: files; Name: "{app}\client\Qt5Network.dll"
Type: files; Name: "{app}\client\Qt5Svg.dll"
Type: files; Name: "{app}\client\Qt5WebSockets.dll"
Type: files; Name: "{app}\client\Qt5Widgets.dll"
Type: files; Name: "{app}\client\bearer\qgenericbearer.dll"
Type: files; Name: "{app}\client\imageformats\qtiff.dll"
Type: files; Name: "{app}\client\imageformats\qwbmp.dll"
Type: files; Name: "{app}\client\imageformats\qwebp.dll"
Type: files; Name: "{app}\client\imageformats\qicns.dll"
Type: files; Name: "{app}\client\imageformats\qtga.dll"

[UninstallRun]
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic server (TCP)"""; Flags: runhidden; Check: InstallServer; Tasks: firewall_traintastic
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic server (UDP)"""; Flags: runhidden; Check: InstallServer; Tasks: firewall_traintastic
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic server (WLANmaus/Z21)"""; Flags: runhidden; Check: InstallServer; Tasks: firewall_wlanmaus
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic simulator (TCP)"""; Flags: runhidden; Check: InstallSimulator; Tasks: firewall_traintastic_simulator

[Icons]
; Server
Name: "{autoprograms}\{#Name}\{#Name} server"; Filename: "{app}\server\{#ServerExeName}"; Parameters: "--tray"; Check: InstallServer
Name: "{autodesktop}\{#Name} server"; Filename: "{app}\server\{#ServerExeName}"; Parameters: "--tray"; Tasks: desktopicon; Check: InstallServer
; Client
Name: "{autoprograms}\{#Name}\{#Name} client"; Filename: "{app}\client\{#ClientExeName}"; Check: InstallClient
Name: "{autodesktop}\{#Name} client"; Filename: "{app}\client\{#ClientExeName}"; Tasks: desktopicon; Check: InstallClient
; Simulator
Name: "{autoprograms}\{#Name}\{#Name} simulator"; Filename: "{app}\simulator\{#SimulatorExeName}"; Check: InstallSimulator

[Registry]
Root: HKLM; Subkey: "{#CompanySubKey}"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "{#AppSubKey}"; Flags: uninsdeletekey

[INI]
Filename: {commonappdata}\traintastic\traintastic-client.ini; Section: general_; Key: language; String: {code:GetTraintasticLanguage}; Flags: uninsdeleteentry uninsdeletesectionifempty;

[Code]
const
  ComponentsValueName = 'Components';
  ComponentServer = $1;
  ComponentClient = $2;
  ComponentSimulator = $4;
var
  Components : DWORD;
  ComponentsPage : TWizardPage;
  ClientAndServerRadioButton : TRadioButton;
  ClientOnlyRadioButton : TRadioButton;
  SimulatorCheckBox : TCheckBox;

function InstallClient : Boolean;
begin
  Result := (Components and ComponentServer) <> 0;
end;

function InstallServer : Boolean;
begin
  Result := (Components and ComponentClient) <> 0;
end;

function InstallSimulator : Boolean;
begin
  Result := (Components and ComponentSimulator) <> 0;
end;

procedure RegWriteTraintasticComponents(Value: DWORD);
begin
  RegWriteDWORDValue(HKEY_LOCAL_MACHINE, '{#AppSubKey}', ComponentsValueName, Value);
end;

function RegReadTraintasticComponents: DWORD;
begin
  if not RegQueryDWORDValue(HKEY_LOCAL_MACHINE, '{#AppSubKey}', ComponentsValueName, Result) then begin
    Result := 0;
  end;
end;

procedure ComponentsPageUpdateNextButtonEnabled;
begin
  Wizardform.NextButton.Enabled := ClientAndServerRadioButton.Checked or ClientOnlyRadioButton.Checked;
end;

procedure ComponentRadioButtonClick(Sender: TObject);
begin
  ComponentsPageUpdateNextButtonEnabled;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = ComponentsPage.ID then
    ComponentsPageUpdateNextButtonEnabled;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  if CurPageID = ComponentsPage.ID then begin
    Components := 0;
    
    if ClientAndServerRadioButton.Checked then
      Components := Components or ComponentServer or ComponentClient
    else if ClientOnlyRadioButton.Checked then
      Components := Components or ComponentClient;
    
    if SimulatorCheckBox.Checked then
      Components := Components or ComponentSimulator;
    
    RegWriteTraintasticComponents(Components);
  end;
  Result := True;
end;

procedure InitializeWizard;
var
  Lbl: TLabel;
begin
  Components := RegReadTraintasticComponents;

  ComponentsPage := CreateCustomPage(wpSelectComponents, SetupMessage(msgWizardSelectComponents), SetupMessage(msgSelectComponentsDesc));

  ClientAndServerRadioButton := TNewRadioButton.Create(ComponentsPage);
  ClientAndServerRadioButton.Caption := ExpandConstant('{cm:client_and_server}');
  ClientAndServerRadioButton.Checked := InstallClient and InstallServer;
  ClientAndServerRadioButton.Font.Style := [fsBold];
  ClientAndServerRadioButton.Height := ScaleY(23);
  ClientAndServerRadioButton.Width := ComponentsPage.SurfaceWidth;
  ClientAndServerRadioButton.Parent := ComponentsPage.Surface;
  ClientAndServerRadioButton.OnClick := @ComponentRadioButtonClick;

  Lbl := TLabel.Create(ComponentsPage);
  Lbl.Caption := ExpandConstant('{cm:Client_and_server_desc}');
  Lbl.Top := ClientAndServerRadioButton.Top + ClientAndServerRadioButton.Height;
  Lbl.Left := ScaleX(17);
  Lbl.Height := ScaleY(23);
  Lbl.Parent := ComponentsPage.Surface;

  ClientOnlyRadioButton := TNewRadioButton.Create(ComponentsPage);
  ClientOnlyRadioButton.Caption := ExpandConstant('{cm:client_only}');
  ClientOnlyRadioButton.Checked := InstallClient and not InstallServer;
  ClientOnlyRadioButton.Font.Style := [fsBold];
  ClientOnlyRadioButton.Top := Lbl.Top + Lbl.Height + ScaleY(10);
  ClientOnlyRadioButton.Height := ScaleY(23);
  ClientOnlyRadioButton.Width := ComponentsPage.SurfaceWidth;
  ClientOnlyRadioButton.Parent := ComponentsPage.Surface;
  ClientOnlyRadioButton.OnClick := @ComponentRadioButtonClick;

  Lbl := TLabel.Create(ComponentsPage);
  Lbl.Caption := ExpandConstant('{cm:client_only_desc}');
  Lbl.Top := ClientOnlyRadioButton.Top + ClientOnlyRadioButton.Height;
  Lbl.Left := ScaleX(17);
  Lbl.Height := ScaleY(23);
  Lbl.Parent := ComponentsPage.Surface;
  
  SimulatorCheckBox := TNewCheckBox.Create(ComponentsPage);
  SimulatorCheckBox.Caption := ExpandConstant('{cm:simulator}');
  SimulatorCheckBox.Checked := InstallSimulator;
  SimulatorCheckBox.Font.Style := [fsBold];
  SimulatorCheckBox.Top := Lbl.Top + Lbl.Height + ScaleY(30);
  SimulatorCheckBox.Height := ScaleY(23);
  SimulatorCheckBox.Width := ComponentsPage.SurfaceWidth;
  SimulatorCheckBox.Parent := ComponentsPage.Surface;
  
  Lbl := TLabel.Create(ComponentsPage);
  Lbl.Caption := ExpandConstant('{cm:simulator_desc}');
  Lbl.Top := SimulatorCheckBox.Top + SimulatorCheckBox.Height;
  Lbl.Left := ScaleX(17);
  Lbl.Height := ScaleY(23);
  Lbl.Parent := ComponentsPage.Surface;
end;

function GetTraintasticLanguage(Param: String) : String;
begin
  case ActiveLanguage of
    'nl': Result := 'nl-nl';
    'de': Result := 'de-de';
    'it': Result := 'it-it';
    'sv': Result := 'sv-se';
    'fr': Result := 'fr-fr';
  else
    Result := 'en-us';
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ServerSettingsFile: String;
begin
  if CurStep = ssPostInstall then begin
    // Server: only write language if there is no setting file yet:
    ServerSettingsFile := ExpandConstant('{localappdata}\traintastic\server\settings.json');
    if not FileExists(ServerSettingsFile) then begin
      SaveStringToFile(ServerSettingsFile, '{"language":"' + GetTraintasticLanguage('') + '"}', False);
    end;
  end
end;

function VC2019RedistNeedsInstall: Boolean;
var
  Version: String;
begin
  if RegQueryStringValue(HKEY_LOCAL_MACHINE,
       'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version) then
  begin
    // Is the installed version at least 14.24 ?
    Log('VC Redist Version check : found ' + Version);
    Result := (CompareStr(Version, 'v14.24.28127.04')<0);
  end
  else
  begin
    // Not even an old version installed
    Result := True;
  end;
  if (Result) then
  begin
    ExtractTemporaryFile('vc_redist.x64.exe');
  end;
end;
