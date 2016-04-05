param (
  [Parameter(Mandatory=$True,Position=1)]
  [string]$jazzDir,
	
  [Parameter(Position=2)]
  [string]$jazzTSFDir,
	
  [Parameter(Position=3)]
  [string]$pclcDir
)

# Usage needs PsIni from PsGet:
#   (new-object Net.WebClient).DownloadString("http://psget.net/GetPsGet.ps1") | iex
#   Install-Module PsIni
# First time also probably requires:
#   Set-ExecutionPolicy -Scope CurrentUser Unrestricted

$levels = @(
#    Target name                           File         Nxt Scr
    ('011_castle1_dungeondilemma'        , 'Castle1'   , 1 , 99), # 0
    ('012_castle2_knightcap'             , 'Castle1n'  , 2 , 99), # 1
    ('013_carrotus1_tossedsalad'         , 'Carrot1'   , 3 , 99), # 2
    ('014_carrotus2_carrotjuice'         , 'Carrot1n'  , 4 , 99), # 3
    ('015_labrat1_weirderscience'        , 'Labrat1'   , 5 , 99), # 4
    ('016_labrat2_loosescrews'           , 'Labrat2'   , 7 , 6 ), # 5
    ('019_labrat3_returnofbirdland'      , 'Labrat3'   , 5 , 99), # 6
    ('021_colon1_victoriansecret'        , 'Colon1'    , 8 , 99), # 7
    ('022_colon2_colonialchaos'          , 'Colon2'    , 9 , 99), # 8
    ('023_psych1_purplehazemaze'         , 'Psych1'    , 10, 99), # 9
    ('024_psych2_funkygrooveathon'       , 'Psych2'    , 11, 13), # 10
    ('025_beach1_beachbunnybingo'        , 'Beach'     , 12, 99), # 11
    ('026_beach2_marinatedrabbit'        , 'Beach2'    , 14, 99), # 12
    ('029_psych3_farout'                 , 'Psych3'    , 10, 99), # 13
    ('031_diamondus1_adiamondusforever'  , 'Diam1'     , 15, 99), # 14
    ('032_diamondus2_fourteencarrot'     , 'Diam3'     , 16, 99), # 15
    ('033_tubelectric1_electricboogaloo' , 'Tube1'     , 17, 99), # 16
    ('034_tubelectric2_voltagevillage'   , 'Tube2'     , 18, 99), # 17
    ('035_medivo1_medievalkineval'       , 'Medivo1'   , 19, 20), # 18
    ('036_medivo2_harescare'             , 'Medivo2'   , 21, 99), # 19
    ('039_medivo3_gargoyleslair'         , 'Garglair'  , 18, 99), # 20
    ('041_jungle1_thrillergorilla'       , 'Jung1'     , 22, 99), # 21
    ('042_jungle2_junglejump'            , 'Jung2'     , 23, 99), # 22
    ('043_inferno1_acolddayinheck'       , 'Hell'      , 24, 99), # 23
    ('044_inferno2_rabbitroast'          , 'Hell2'     , 25, 99), # 24
    ('045_damn1_burninbiscuits'          , 'Damn1'     , 26, 99), # 25
    ('046_damn2_badpitt'                 , 'Damn2'     , 99, 99), # 26
    ('051_labrat_darnratz'               , 'Share1'    , 28, 99), # 27
    ('052_psych_retrorabbit'             , 'Share2'    , 29, 99), # 28
    ('053_diamondus_frogstomp'           , 'Share3'    , 99, 99), # 29
    ('061_battle1'                       , 'Battle1'   , 31, 99), # 30
    ('062_battle2'                       , 'Battle2'   , 32, 99), # 31
    ('063_battle3'                       , 'Battle3'   , 30, 99), # 32
    ('071_race1'                         , 'Race1'     , 34, 99), # 33
    ('072_race2'                         , 'Race2'     , 35, 99), # 34
    ('073_race3'                         , 'Race3'     , 33, 99), # 35
    ('081_ctf1'                          , 'Capture1'  , 37, 99), # 36
    ('082_ctf2'                          , 'Capture2'  , 38, 99), # 37
    ('083_ctf3'                          , 'Capture3'  , 36, 99), # 38
    ('091_treasure1'                     , 'Treasur1'  , 40, 99), # 39
    ('092_treasure2'                     , 'Treasur2'  , 41, 99), # 40
    ('093_treasure3'                     , 'Treasur3'  , 39, 99), # 41
    ('010_castle3_rabbitintraining'      , 'Trainer'   , 0 , 99)  # 42
)
$tsflevels = @(                            
    ('201_easter1_easterbunny'           , 'Easter1'   , 1 , 99), # 0
    ('202_easter2_springchickens'        , 'Easter2'   , 2 , 99), # 1
    ('203_easter3_scrambledeggs'         , 'Easter3'   , 3 , 99), # 2
    ('204_haunted1_ghostlyantics'        , 'Haunted1'  , 4 , 99), # 3
    ('205_haunted2_skeletonsturf'        , 'Haunted2'  , 5 , 99), # 4
    ('206_haunted3_graveyardshift'       , 'Haunted3'  , 6 , 99), # 5
    ('207_town1_turtletown'              , 'Town1'     , 7 , 99), # 6
    ('208_town2_suburbiacommando'        , 'Town2'     , 8 , 99), # 7
    ('209_town3_urbanbrawl'              , 'Town3'     , 99, 99), # 8
    ('210_battle1'                       , 'Abattle1'  , 9 , 99), # 9
    ('211_race1'                         , 'Arace1'    , 11, 99), # 10
    ('212_race2'                         , 'Arace2'    , 10, 99), # 11
    ('299_queenofboard'                  , 'Ml_qob'    , 99, 99)  # 12
)

if ($pclcDir -eq '') {
    $pclcDir = "..\Debug\x64\"
}

If (!(Test-Path "$pclcDir\pclc.exe")) {
    echo "The executable path '$pclcDir\pclc.exe' is incorrect."
    exit
}

function Convert-Game {
    param(
        $dir,
        $dirlevels
    )
    
    foreach ($level in $dirlevels) {
        $targetname = $level[0]
        $sourcename = $level[1]
        $sourcefile = "$dir\$sourcename.j2l"
        
        echo "Converting '$sourcefile'..."
        
        # TODO: relocate old folder if exists. Right now the new
        # version is generated with the different name in case of collision,
        # but the ini editing still edits as if everything went as normal.
        
        echo '`n' | & "$pclcDir/pclc.exe" ($targetname, $sourcefile)
        
        if ($LASTEXITCODE -eq 0) {
            echo "Updating configuration."
            $ini = Get-IniContent "$targetname\config.ini"
            if ($level[2] -ne 99) {
                $nextname = $dirlevels[$level[2]][0]
                $ini["Level"]["Next"] = $nextname
            }
            if ($level[3] -ne 99) {
                $secretname = $dirlevels[$level[3]][0]
                $ini["Level"]["Secret"] = $secretname
            }
            Out-IniFile -FilePath "$targetname\config.ini" -InputObject $ini -Force
        } else {
            echo "Failed."
        }
    }
}

Convert-Game $jazzDir $levels
if ($jazzTSFDir -ne '') {
    Convert-Game $jazzTSFDir $tsflevels
}
