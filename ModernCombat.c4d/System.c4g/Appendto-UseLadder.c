/*-- Leiter-Funktionalitšt --*/

//Erweitert die Leiter-Kletterfunktion um das Herunterrutschen per [Doppelstop].

#strict 2
#appendto L_LA


private func ControlLadder(string strControl, par1)
{
  //Wenn schnell genug, wieder an Leiter anhalten
  if(lastladder)
    lastladder = 0;

  //Nicht an einem kletterbaren Objekt
  if(GetAction() != "ScaleLadder") 
    return;

  //Klettert an der Leiter
  if(strControl == "ControlCommand")
  {
    ClimbLadder();
    return(1);
  }

  var effect;
  if((effect = GetEffect("ScalingLadder", this)) && strControl != "ControlDown" && par1 != COMD_Down)
    EffectVar(1, this, effect) = false;

  //Log("%s: %s",GetName(),strControl); //DEBUGZ!

  // JnR
  if (strControl == "ControlUpdate")
  {
    var comdir = par1;
    // Strip away COMD_Left/COMD_Right, this is handled via ControlLeft/ControlRight
    if(ComDirLike(comdir, COMD_Left)) comdir = ComDirTransform(comdir, COMD_Right);
    if(ComDirLike(comdir, COMD_Right)) comdir = ComDirTransform(comdir, COMD_Left);
    SetComDir(comdir);

    if(comdir == COMD_Down)
    {
      if(GetPlrDownDouble(GetOwner()) && CanSlideLadder())
      {
        EffectVar(1, this, effect) = true;
        Sound("LADR_Slide.ogg", 0, this, 40, 0, +1);
      }
    }
    else
      Sound("LADR_Slide.ogg", 0, this, 40, 0, -1);
  }

  //Rauf
  if(strControl == "ControlUp" && !GetPlrCoreJumpAndRunControl(GetController()))
  {
    SetComDir(COMD_Up);
    Sound("LADR_Slide.ogg", 0, this, 40, 0, -1);
  }
  //Runter
  if(strControl == "ControlDown" && !GetPlrCoreJumpAndRunControl(GetController()))
    SetComDir(COMD_Down);
  //Rechts
  if(strControl == "ControlRight")
  {
    //Richtung wechseln
    if(GetDir() == DIR_Right)
    {
      SetDir(DIR_Left);
      SetComDir(COMD_Stop);
      if(!AdjustLadderOffset(GetActionTarget()))
        SetDir(DIR_Right);
      Sound("LADR_Slide.ogg", 0, this, 40, 0, -1);
    }
    //Abspringen
    else
    {
      if(GetComDir() == COMD_Up) ReleaseLadderUp();
      else if(GetComDir() == COMD_Stop) ReleaseLadderStop();
      else ReleaseLadderDown();
    }
  }
  //Links
  if(strControl == "ControlLeft")
  {
    //Richtung wechseln
    if(GetDir() == DIR_Left)
    {
      SetDir(DIR_Right);
      SetComDir(COMD_Stop);
      if(!AdjustLadderOffset(GetActionTarget()))
        SetDir(DIR_Left);
      Sound("LADR_Slide.ogg", 0, this, 40, 0, -1);
    }
    //Loslassen
    else
    {
      if(GetComDir() == COMD_Up) ReleaseLadderUp();
      else if(GetComDir() == COMD_Stop) ReleaseLadderStop();
      else ReleaseLadderDown();
    }
  }

  //Runterrutschen
  if(strControl == "ControlDownDouble" && effect && CanSlideLadder())
  {
    SetComDir(COMD_Down);
    EffectVar(1, this, effect) = true;
    Sound("LADR_Slide.ogg", 0, this, 40, 0, +1);
  }

  //Werfen
  if(strControl == "ControlThrow") LadderThrow();

  //Graben
  if(strControl == "ControlDig") LadderDig();

  return(1);
}

public func CanSlideLadder()
{
  if(GetPhysical("Scale", 0, this) < GetPhysical("Scale", 0, 0, GetID()))
    return false;

  return true;
}

protected func FxScalingLadderTimer(object pTarget, int iEffectNumber)
{
  var pLadder;
  //Kletteraktion verloren: Abbruch
  if(!(GetAction() == "ScaleLadder"))
    //Klettern beenden
    return(ReleaseLadder(0));

  //Keine Leiter mehr: Klettern beenden
  if(!(pLadder = FindLadder()))
  {
    //Oberes Ende: Versuchen, die Wand zu erreichen
    if(GetComDir() == COMD_Up)
    {
      return(ReleaseLadder(-15 + GetDir() * 30,-20));
    }
    //Unteres Ende: Fallenlassen
    else
      return(ReleaseLadder(0));
  }

  //Gegebenenfalls neues Objekt zum klettern speichern
  SetActionTargets(pLadder);
  //Abstand zum Objekt
  AdjustLadderOffset(pLadder);

  //Klettergeschwindigkeit 
  var iStep = Max(1,(GetPhysical("Scale")*4*GetLadderSpeed())/20000);

  //Kletterbewegung
  var iPosY = EffectVar(0, pTarget, iEffectNumber);
  var iLastY = GetY();
  var iPhase = GetPhase();
  var fSliding = EffectVar(1, pTarget, iEffectNumber);
  if(fSliding && !CanSlideLadder())
  {
    Sound("LADR_Slide.ogg", 0, this, 40, 0, -1);
    fSliding = EffectVar(1, pTarget, iEffectNumber) = 0;
  }
  if(GetComDir() == COMD_Up)
  {
    iPosY -= iStep;
    iPhase += iStep*14/1000;

    if(!fSliding)
      Sound(Format("ClonkClimb%s*.ogg", pLadder->~GetGraphicsName()), 0, 0, 50);
  }
  if(GetComDir() == COMD_Down)
  {
    var iPosX = GetX();
    var inewPosY = iPosY +(iStep*5)/2;
    var iYAdd = GetObjectVal("Height", 0, this) + GetObjectVal("Offset", 0, this, 1);
    if(fSliding && PathFree(iPosX, iLastY + iYAdd, iPosX, inewPosY/100 + iYAdd))
    {
      iPosY = inewPosY;
      if(GetEffectData(EFSM_ExplosionEffects) > 0) 
        CastSmoke("Smoke3",RandomX(2,4),RandomX(2,8),(GetDir(pTarget)*2-1)*6,2,4,70);
    }
    else
    {
      iPosY += iStep;
      iPhase -= iStep*14/1000;

      Sound(Format("ClonkClimb%s*.ogg", pLadder->~GetGraphicsName()), 0, 0, 50);
    }
  }

  //Stecken wir fest?
  if(Stuck())
  {
    //Nicht in festes Material klettern
    if(GetComDir() == COMD_Up)
    {
      ReleaseLadder(0,-2);
      SetAction("Hangle");
    }
    if(GetComDir() == COMD_Down)
    {
      ReleaseLadder(0,2);
      SetAction("Walk");
    }
    else
    {
      SetPosition(GetX(), iLastY);
      SetComDir(COMD_Stop);
    }
    return;
  }

  if(iPhase < 0) iPhase = 15; if(iPhase > 15) iPhase = 0;
  SetPhase(iPhase);
  SetPosition(GetX(), iPosY/100);
  SetXDir(0); SetYDir(0);

  EffectVar(0, pTarget, iEffectNumber) = iPosY;
}

public func ReleaseLadder(int iXDir, int iYDir)
{
  var dir;

  var diffx, pLadder = GetActionTarget(), xpos = GetX(pLadder);
  //Leiter noch vorhanden?
  if(!pLadder)
    return;

  lastladder = pLadder;

  Sound("LADR_Slide.ogg", 0, this, 40, 0, -1);

  //Klettereffekt beenden
  RemoveEffect("ScalingLadder", this);

  //und runter von der Leiter
  if(dir < 0 || GetContact(0,1,CNAT_Bottom))
  {
    SetAction("Walk");
  }
  if(dir > 0)
  {
    SetAction("Hangle");
  }
  else
    if(GetAction() != "Tumble")
      SetAction("Jump");

  SetXDir(iXDir);
  SetYDir(iYDir);

  return(1);
}