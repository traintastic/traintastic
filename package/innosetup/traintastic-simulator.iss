; Inno Setup Script for Traintastic simulator

#define VersionInfoBinary "..\..\simulator\build\Release\traintastic-simulator.exe"

#define Name "Traintastic simulator"
#define Version GetFileVersion(VersionInfoBinary)
#define VersionFull GetStringFileInfo(VersionInfoBinary, FILE_VERSION)
#define Copyright GetFileCopyright(VersionInfoBinary)
#define Publisher "Reinder Feenstra"
#define URL "https://traintastic.org"
#define SimulatorExeName "traintastic-simulator.exe"

[Setup]
AppId={{18733815-6689-45A7-BEE9-B9089A41CC42}
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
OutputBaseFilename=traintastic-simulator-setup-v{#VersionFull}
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
MinVersion=10.0

[Languages]
Name: en; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Simulator
Source: "..\..\simulator\build\Release\{#SimulatorExeName}"; DestDir: "{app}\simulator"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\*.dll"; DestDir: "{app}\simulator"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\generic\*.dll"; DestDir: "{app}\simulator\generic"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\iconengines\*.dll"; DestDir: "{app}\simulator\iconengines"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\imageformats\*.dll"; DestDir: "{app}\simulator\imageformats"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\networkinformation\*.dll"; DestDir: "{app}\simulator\networkinformation"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\platforms\*.dll"; DestDir: "{app}\simulator\platforms"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\styles\*.dll"; DestDir: "{app}\simulator\styles"; Flags: ignoreversion
Source: "..\..\simulator\build\Release\tls\*.dll"; DestDir: "{app}\simulator\tls"; Flags: ignoreversion
; Simulator layouts
Source: "..\..\simulator\layout\*.json"; DestDir: "{commonappdata}\traintastic\layout"; Flags: ignoreversion
; VC++ redistributable runtime. Extracted by VC2019RedistNeedsInstall(), if needed.
Source: "..\..\simulator\build\Release\vc_redist.x64.exe"; DestDir: {tmp}; Flags: dontcopy

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; StatusMsg: "Installing VC++ redistributables..."; Parameters: "/quiet /norestart"; Check: VC2019RedistNeedsInstall; Flags: waituntilterminated

[Icons]
; Simulator
Name: "{autoprograms}\{#Name}\{#Name} simulator"; Filename: "{app}\simulator\{#SimulatorExeName}"

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
