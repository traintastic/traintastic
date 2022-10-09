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

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Types]
Name: "clientandserver"; Description: "Client and Server"
Name: "clientonly"; Description: "Client only"

[Components]
Name: "server"; Description: "Traintastic server"; Types: clientandserver
Name: "client"; Description: "Traintastic client"; Types: clientandserver clientonly; Flags: fixed

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "firewall_traintastic"; Description: "Allow Traintastic client"; GroupDescription: "Windows Firewall:"; Components: server
Name: "firewall_wlanmaus"; Description: "Allow WLANmaus/Z21"; GroupDescription: "Windows Firewall:"; Components: server

[Files]
; Server
Source: "..\..\server\build\{#ServerExeName}"; DestDir: "{app}\server"; Flags: ignoreversion; Components: server
Source: "..\..\server\thirdparty\lua5.3\bin\win64\lua53.dll"; DestDir: "{app}\server"; Flags: ignoreversion; Components: server
Source: "..\..\server\thirdparty\libarchive\bin\archive.dll"; DestDir: "{app}\server"; Flags: ignoreversion; Components: server
; Client
Source: "..\..\client\build\Release\{#ClientExeName}"; DestDir: "{app}\client"; Flags: ignoreversion; Components: client
Source: "..\..\client\build\Release\*.dll"; DestDir: "{app}\client"; Flags: ignoreversion; Components: client
Source: "..\..\client\build\Release\bearer\*.dll"; DestDir: "{app}\client\bearer"; Flags: ignoreversion; Components: client
Source: "..\..\client\build\Release\iconengines\*.dll"; DestDir: "{app}\client\iconengines"; Flags: ignoreversion; Components: client
Source: "..\..\client\build\Release\imageformats\*.dll"; DestDir: "{app}\client\imageformats"; Flags: ignoreversion; Components: client
Source: "..\..\client\build\Release\platforms\*.dll"; DestDir: "{app}\client\platforms"; Flags: ignoreversion; Components: client
Source: "..\..\client\build\Release\styles\*.dll"; DestDir: "{app}\client\styles"; Flags: ignoreversion; Components: client
; Shared
Source: "..\..\shared\translations\*.txt"; DestDir: "{commonappdata}\traintastic\translations"; Flags: ignoreversion;
; Manual
Source: "..\..\manual\build\*"; DestDir: "{commonappdata}\traintastic\manual"; Flags: ignoreversion recursesubdirs
; LNCV XML
Source: "..\..\shared\data\lncv\xml\*.xml"; DestDir: "{commonappdata}\traintastic\lncv"; Flags: ignoreversion; Components: client
Source: "..\..\shared\data\lncv\xml\lncvmodule.xsd"; DestDir: "{commonappdata}\traintastic\lncv"; Flags: ignoreversion; Components: client
; VC++ redistributable runtime. Extracted by VC2019RedistNeedsInstall(), if needed.
Source: "..\..\client\build\Release\vc_redist.x64.exe"; DestDir: {tmp}; Flags: dontcopy

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; StatusMsg: "Installing VC++ redistributables..."; Parameters: "/quiet /norestart"; Check: VC2019RedistNeedsInstall; Flags: waituntilterminated; Components: client
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic server (TCP)"" dir=in program=""{app}\server\{#ServerExeName}"" protocol=TCP localport=5740 action=allow"; StatusMsg: "Add firewall rule for Traintastic client (TCP)"; Flags: runhidden; Components: server; Tasks: firewall_traintastic
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic server (UDP)"" dir=in program=""{app}\server\{#ServerExeName}"" protocol=UDP localport=5740 action=allow"; StatusMsg: "Add firewall rule for Traintastic client (UDP)"; Flags: runhidden; Components: server; Tasks: firewall_traintastic
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Traintastic server (WLANmaus/Z21)"" dir=in program=""{app}\server\{#ServerExeName}"" protocol=UDP localport=21105 action=allow"; StatusMsg: "Add firewall rule for WLANmaus/Z21"; Flags: runhidden; Components: server; Tasks: firewall_wlanmaus

[UninstallRun]
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic server (TCP)"""; Flags: runhidden; Components: server; Tasks: firewall_traintastic
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic server (UDP)"""; Flags: runhidden; Components: server; Tasks: firewall_traintastic
Filename: {sys}\netsh.exe; Parameters: "advfirewall firewall delete rule name=""Traintastic server (WLANmaus/Z21)"""; Flags: runhidden; Components: server; Tasks: firewall_wlanmaus

[Icons]
; Server
Name: "{autoprograms}\{#Name}\{#Name} server"; Filename: "{app}\server\{#ServerExeName}"; Parameters: "--tray"; Components: server
Name: "{autodesktop}\{#Name} server"; Filename: "{app}\server\{#ServerExeName}"; Parameters: "--tray"; Tasks: desktopicon; Components: server
; Client
Name: "{autoprograms}\{#Name}\{#Name} client"; Filename: "{app}\client\{#ClientExeName}"; Components: client
Name: "{autodesktop}\{#Name} client"; Filename: "{app}\client\{#ClientExeName}"; Tasks: desktopicon; Components: client

[Code]
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
