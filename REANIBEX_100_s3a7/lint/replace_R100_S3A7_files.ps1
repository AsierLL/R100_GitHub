#Requires -Version 3.0
<#
.SYNOPSIS
  Script to replace a String inside a file
.DESCRIPTION
  This script replaces a given string (usually a path) in the textfile <R100_S3A7_files.lnt> with the string %PROJ_PATH%
  This is done to mantain a clear and neutral version of the file <R100_S3A7_files.lnt> which is needed for the PC-Lint execution
  under the project R100_S3A7
  This script is part of the Lint Build phase of the project, executed by the Renesas e2studio IDE Version: 6.2.0
  The function replace_PS_version is not actually beingused. To use this function, remove the #Requires statement 
  from the header of this file. Otherwise, the script will fail
.PARAMETER OldStr
  Mandatory. String to be replaced
.PARAMETER LintPath
  Mandatory. Path to the lint files directory
.INPUTS
  Parameters above
.OUTPUTS
  Log file stored in LintPath\replace_R100_S3A7.log
.NOTES
  Version:        1.0
  Author:         jarribas@bexencardio.com
  Creation Date:  15/05/2018
  Purpose/Change: Initial script development
  
.EXAMPLE
  Powershell.exe -executionpolicy remotesigned -File  replace_R100_S3A7_files.ps1 $OldStr $LintPath
  Powershell.exe -executionpolicy remotesigned -File  C:\Reanibex_Serie_100\TRUNK\04_IMPLEMENTATION\SOFTWARE\TRUNK\04_IMPLEMENTATION\R100_S3A7\lint\replace_R100_S3A7_files.ps1 C:\Reanibex_Serie_100\TRUNK\04_IMPLEMENTATION\SOFTWARE\TRUNK\04_IMPLEMENTATION\R100_S3A7 C:\Reanibex_Serie_100\TRUNK\04_IMPLEMENTATION\SOFTWARE\TRUNK\04_IMPLEMENTATION\R100_S3A7\lint
#>
#---------------------------------------------------------[Initialisations]--------------------------------------------------------

#Set Input Parameters
param ([string]$OldStr, [string]$LintPath)

#Set Lint folder path as current path
Set-Location -Path $LintPath

#Set Error Action to Silently Continue
$ErrorActionPreference = "SilentlyContinue"

#Dot Source required Function Libraries
. "$LintPath\Logging_Functions.ps1"

#----------------------------------------------------------[Declarations]----------------------------------------------------------

#Script Version
$sScriptVersion = "1.0"

#Log File Info
$sLogPath = $LintPath
$sLogName = "replace_R100_S3A7.log"
$sLogFile = Join-Path -Path $sLogPath -ChildPath $sLogName

#Modify string to be replaced
$ModStr = $OldStr.replace('/', '\')

#-----------------------------------------------------------[Functions]------------------------------------------------------------

Function replace_PS_version{
  <#
  .SYNOPSIS
    Replaces a given string with the string %PROJ_PATH%, depending on the PowerShell version instaled
  .DESCRIPTION
    Replaces a given string (usually a path) in the textfile <R100_S3A7_files.lnt> with the string %PROJ_PATH%.
	Depending on the PowerShell version instaled, executes the replace command for V2 or V3
  .PARAMETER LogPath
    None
  .INPUTS
    None
  .OUTPUTS
    Log file stored in LintPath\replace_R100_S3A7.log
  .NOTES
    Version:        1.0
    Author:         jarribas@bexencardio.com
    Creation Date:  15/05/2018
    Purpose/Change: Initial function development
  .EXAMPLE
    replace_PS_version
  #>
  param ([string]$OldStr, [string]$LintPath)
  
  Begin{
    Log-Write -LogPath $sLogFile -LineValue "<description of what is going on>..."
  }
  
  Process{
    Try{
      $ModStr = $OldStr.replace('/', '\')
		Set-Location -Path $LintPath
		#Write-Host "$ModStr"
		#Write-Host (Get-Host).version
		if ($PSVersionTable.PSVersion.Major -gt 2)
		{
			# -Version 3.0
			(Get-Content R100_S3A7_files.lnt).replace($ModStr, '%PROJ_PATH%') | Set-Content R100_S3A7_files.lnt
		}
		else
		{
			# -Version 2.0
			(Get-Content R100_S3A7_files.lnt) -replace $ModStr, '%PROJ_PATH%' | Set-Content R100_S3A7_files.lnt
		}
    }
    
    Catch{
      Log-Error -LogPath $sLogFile -ErrorDesc $_.Exception -ExitGracefully $True
      Break
    }
  }
  
  End{
    If($?){
      Log-Write -LogPath $sLogFile -LineValue "Completed Successfully."
      Log-Write -LogPath $sLogFile -LineValue " "
    }
  }
}

#-----------------------------------------------------------[Execution]------------------------------------------------------------
Log-Start -LogPath $sLogPath -LogName $sLogName -ScriptVersion $sScriptVersion
Log-Write -LogPath $sLogFile -LineValue "Start Replacing process"
(Get-Content R100_S3A7_files.lnt).replace($ModStr, '%PROJ_PATH%') | Set-Content R100_S3A7_files.lnt
Log-Write -LogPath $sLogFile -LineValue "Finished Replacing process"
Log-Finish -LogPath $sLogFile