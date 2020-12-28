/*-- Zielen vor anfassbaren Objekten --*/

//Erleichtert das Zielen per [Dreifachstopp] vor anfassbaren Objekten, sofern der Spieler es für sich in den Einstellungen aktiviert hat.

#strict 3
#appendto *


/* Steuerung */

public func ControlDown(object pByObj)
{
  if(!(GetOCF() & OCF_Grab) || !pByObj)
    return(_inherited(pByObj, ...));

  //Bei Dreifachstop loslassen und Zielen
  if(pByObj && pByObj->~BetterAiming() && !GetPlrCoreJumpAndRunControl(GetController(pByObj)) && GetEffect("DidGrab", pByObj) && Contents(0, pByObj))
  {
    pByObj->SetAction("Walk");
    if(pByObj->~ReadyToSquatAim())
      pByObj->~StartSquatAiming();

    return;
  }

  return(_inherited(pByObj, ...));
}

public func Grabbed(object pByObj, bool fGrab)
{
  if(pByObj && fGrab && !this()->~NoFastAiming())
  {
    var timer = 10;
    if (GetPlrCoreJumpAndRunControl(GetController(pByObj)))
      timer = 20; // JnR players need 2 presses because they only pressed once to grab
    AddEffect("DidGrab", pByObj, 100, timer);
  }

  return _inherited(pByObj, fGrab, ...);
}