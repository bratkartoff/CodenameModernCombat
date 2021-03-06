/*-- MTP --*/

#strict 2
#include PACK

public func IsDrawable()		{return true;}		//Wird sichtbar getragen
public func HandX()			{return 4000;}
public func HandY()			{return 10;}
public func HandSize()			{return 1000;}

public func StartPoints()		{return 200;}
public func MaxPoints()			{return 200;}
public func RefillTime()		{return 20;}
public func TeamSupportTime()		{return 60;}

public func IsEquipment()		{return !NoAmmo();}
public func MinValue()			{return 49;}
public func MinValue2()			{return 99;}
public func AI_Inventory(object pClonk)	{return true;}


/* Auff�llen */

public func CanRefill()
{
  //Nur wenn von Unterst�tzer-Klasse getragen
  return GetEffect("ClonkClass_Support", Contained());
}

/* Initialisierung */

protected func Initialize()
{
  //Keine Munition-Regel vorhanden? Verschwinden
  if(NoAmmo())
    return ScheduleCall(this, "RemoveObject", 1);

  //Munitionsbalken erstellen
  AddEffect("AmmoBars", this, 1, 1, this);

  return _inherited(...);
}

public func FxAmmoBarsStart(object target, int nr)
{
  EffectVar(0, target, nr) = [];
  return true;
}

public func FxAmmoBarsTimer(object target, int nr)
{
  if(!Contained() || !Contained()->~IsClonk())
  {
    if(!EffectVar(1, target, nr))
    {
      for(var bar in EffectVar(0, target, nr))
        if(bar)
          RemoveObject(bar);

      EffectVar(1, target, nr) = true;
    }
    return 0;
  }
  EffectVar(1, target, nr) = false;

  var owner = GetOwner(Contained());

  //Balken updaten
  for(var bar in EffectVar(0, target, nr))
  {
    if(!bar)
      continue;

    var actTarget = GetActionTarget(0, bar); var weapon;
    //actTarget lebt nicht mehr / ist verfeindet oder sein Spieler existiert nicht mehr? Balken l�schen
    if(!GetPlayerName(GetOwner(actTarget)) || !(GetOCF(actTarget) & OCF_Alive) || Hostile(GetOwner(actTarget), owner))
      RemoveObject(bar);
    //actTarget befindet sich in einem Objekt, hat keine Waffe ausgew�hlt oder hat keinen Munitionsg�rtel: Ausblenden
    else if(Contained(actTarget) || !(weapon = Contents(0, actTarget)) || !weapon->~IsWeapon() || !actTarget->~AmmoStoring())
      bar->Update(0, true);
    else if(weapon->GetFMData(FM_NoAmmoModify, weapon->GetFireTec()))
      bar->Update(0, true);
    else
    {
      //Munitionsdaten einholen
      var ammocount = actTarget->GetAmmo(weapon->GetFMData(FM_AmmoID));
      var ammomax = weapon->GetFMData(FM_AmmoLoad);

      //Falls maximal 1 im Magazin, 10fach als 100%, ansonsten 3fach
      if(ammomax == 1)
        ammomax *= 10;
      else
        ammomax *= 3;

      //Prozentsatz errechnen
      var percent = BoundBy((((100 * 1000) / ammomax) * ammocount) / 1000, 0, 100);

      bar->Update(percent, (percent >= 95));
    }
  }

  //Lebende, im Freien befindliche verb�ndete CrewMember suchen (ausgenommen Container)
  for(var clonk in FindObjects(Find_OCF(OCF_Alive), Find_OCF(OCF_CrewMember), Find_NoContainer(), Find_Exclude(Contained()), Find_Not(Find_Hostile(owner))))
  {
    if(FindObject2(Find_ID(SBAR), Find_ActionTarget(clonk), Find_Owner(owner), Find_Func("HasBarType", BAR_Ammobar))) //Hat schon einen Balken?
      continue;

    var bar = CreateObject(SBAR, 0, 0, owner);
    bar->Set(clonk, RGB(255, 255, 80), BAR_Ammobar, 0, 0, SM11);
    EffectVar(0, target, nr)[GetLength(EffectVar(0, target, nr))] = bar;
  }
  return true;
}

public func FxAmmoBarsStop(object target, int nr)
{
  for(var bar in EffectVar(0, target, nr))
    if(bar)
      RemoveObject(bar);
}

/* Munition entnehmen */

public func AmmoTypes()
{
  //ID, Menge, Punkte
  return [[STAM, 50, 50], [GRAM, 12, 60], [MIAM, 4, 70]];
}

protected func Activate(object pCaller)
{
  //Leeres Pack zerst�ren
  if(!GetPackPoints())
  {
    if(GetEffect("ClonkClass_Support", pCaller))
    {
      PlayerMessage(GetOwner(pCaller), "$CantTake$", pCaller);
      return true;
    }
    Sound("Limitation.ogg", false, this);
    CastParticles("Paper", RandomX(4, 8), 40, 0, 0, 20, 35, RGB(180, 180, 180), RGBa(240, 240, 240, 150));
    RemoveObject();
    return true;
  }
  //Hat schon eine Box
  if(FindContents(CUAM, pCaller))
  {
    PlayerMessage(GetOwner(pCaller), "$NoSpace$", pCaller);
    return true;
  }

  //Falsche Aktion?
  if(!WildcardMatch(GetAction(pCaller), "*Walk*") && !WildcardMatch(GetAction(pCaller), "*Swim*") && !WildcardMatch(GetAction(pCaller), "*Crawl*") && !WildcardMatch(GetAction(pCaller), "*Jump*"))
  {
    PlayerMessage(GetOwner(pCaller), "$CantTake$", pCaller);
    return true;
  }

  //Clonk anhalten
  SetComDir(COMD_Stop, pCaller);

  //Men�
  var aAmmo = AmmoTypes()[0];
  CreateMenu(GetID(), pCaller, this, 0, "$TakeAmmo$", 0, C4MN_Style_Context);
  if(!GetEffect("ClonkClass_Support", pCaller) && GetPackPoints() < aAmmo[2])
  {
    AddMenuItem(Format("<c %x>%d %s</c>", RGB(255,255,51), GetPackPoints(), GetName(0, aAmmo[0])), "CreateAmmoPack", aAmmo[0], pCaller, GetPackPoints(), pCaller, 0, C4MN_Add_ForceNoDesc | 128, 0, i);
  }
  else
  {
    for (var i = 0; i < GetLength(AmmoTypes()); i++)
    {
      var aAmmo = AmmoTypes()[i];
      var iColor = RGB(255,255,51);
      if(GetPackPoints() < aAmmo[2])
        iColor = RGB(119,119,119);
      else if(aAmmo[0]->~MaxAmmo() && GetAmmo(aAmmo[0],pCaller)+aAmmo[1] > aAmmo[0]->~MaxAmmo())
        iColor = RGB(255,255,255);
      AddMenuItem(Format("<c %x>%d %s</c>", iColor, aAmmo[1], GetName(0, aAmmo[0])), "CreateAmmoPack", aAmmo[0], pCaller, aAmmo[2], pCaller, 0, C4MN_Add_ForceNoDesc | 128, 0, i);
    }
  }
  return true;
}

protected func CreateAmmoPack(id idAmmo, object pCaller, bool fRight, int iIndex)
{
  if(!idAmmo || !pCaller)
    return false;

  //Zu wenig Punkte?
  var AmmoCount;
  var aAmmo = AmmoTypes()[iIndex];
  if(GetPackPoints() < aAmmo[2])
  {
    if(!GetEffect("ClonkClass_Support", pCaller))
      AmmoCount = GetPackPoints();
    else
    {
      PlayerMessage(GetOwner(pCaller), "$NeededPoints$", pCaller, aAmmo[2]);
      return false;
    }
  }
  else
    AmmoCount = aAmmo[1];

  //Box erstellen und f�llen
  var box = CreateObject(CUAM, 0, 0, GetOwner(pCaller));
  box->~SetAmmoID(aAmmo[0]);
  box->~SetAmmoCount(AmmoCount, true);
  box->~SetBoxOwner(GetOwner(pCaller));

  //Einsammeln
  if(!Collect(box, pCaller))
  {
    PlayerMessage(GetOwner(pCaller), "$NoSpace$", pCaller);
    return false;
  }

  //Punkte abziehen
  DoPackPoints(-aAmmo[2]);
  Sound("ResupplyOut*.ogg", false, this, 0, GetOwner(pCaller) + 1);

  //Hinwechseln
  ShiftContents(pCaller, 0, CUAM);

  return true;
}

/* Team-Support */

public func DoTeamSupport(array aClonks)
{
  //Zu wenig Punkte
  if(GetPackPoints() < 30)
    return false;

  for (var pTarget in aClonks)
  {
    //Nur Clonks
    if(!pTarget->~IsClonk())
      continue;
      
    //Munitionsbedarf feststellen
    var highestammo = 0, ammoID = 0;
    for(var i = 0; i < ContentsCount(0, pTarget); i++)
      if(Contents(i, pTarget)->~IsWeapon())
        for (var j = 0; j < Contents(i, pTarget)->GetFMCount(); j++)
        {
          var ammocount, weapon = Contents(i, pTarget);
          if(weapon->GetFMData(FM_NoAmmoModify, j)) continue;
          if(weapon->GetFMData(FM_AmmoLoad, j) <= 3)
            ammocount = weapon->GetFMData(FM_AmmoLoad, j) * 10;
          else
          ammocount = weapon->GetFMData(FM_AmmoLoad,j) * 3;
          if(GetAmmo(weapon->GetFMData(FM_AmmoID, j), pTarget) < ammocount)
          {
            if(!ammoID)
              ammoID = weapon->GetFMData(FM_AmmoID,j);
            if(highestammo < ammocount)
              highestammo = ammocount;
          }
        }

    if(!ammoID)
      continue;

    //Munition hinzuf�gen
    var factor = ammoID->~GetPointFactor();
    if(ammoID->MaxAmmo() / 10 * factor > GetPackPoints() || GetAmmo(ammoID, pTarget) >= highestammo)
      continue;

    //Hinweisnachricht: Munition aufgenommen
    HelpMessage(GetOwner(pTarget),"$AmmoReceived$", pTarget, ammoID->MaxAmmo() / 10, ammoID);
    //Nachschubinfo: Munition aufgenommen
    ResupplyInfo(pTarget,ammoID,ammoID->MaxAmmo() / 10);

    DoAmmo(ammoID, ammoID->MaxAmmo()/10, pTarget);
    Sound("ResupplyIn*.ogg",0,pTarget,0,GetOwner(pTarget)+1);
    Sound("ResupplyOut*.ogg");
    DoPackPoints(-ammoID->MaxAmmo() / 10 * factor);

    //Achievement-Fortschritt (Ammo Distributor)
    DoAchievementProgress(ammoID->MaxAmmo() / 10 * factor, AC03, GetOwner(Contained()));

    if(GetOwner() != GetOwner(pTarget))
      //Punkte bei Belohnungssystem (Munitionierung)
      DoPlayerPoints(BonusPoints("Supply", ammoID->MaxAmmo() / 10 * factor), RWDS_TeamPoints, GetOwner(Contained()), Contained(), IC14);
  }
}

/* Sonstiges */

public func ControlThrow()	{return true;}

protected func Hit()
{
  Sound("AmmoBoxHit*.ogg", false, this);
}

protected func Selection()
{
  Sound("FAPK_Charge.ogg", false, this);
}