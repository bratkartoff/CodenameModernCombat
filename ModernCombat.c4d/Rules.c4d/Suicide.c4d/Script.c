/*-- Selbstmord --*/

#strict 2

public func IsChooseable()		{return 1;}	//Kann mittels des Spielzielausw�hlers ausgew�hlt werden
public func RejectRespawnTimer()	{return true;}	//Keine Spawnverz�gerung

/* Initialisiert */

private func Initialized()
{
  //Verschwinden wenn Kein FakeDeath im Spiel ist
  if(FindObject(NOFD))
    RemoveObject();
}

protected func Activate(iPlr)
{
  MessageWindow(GetDesc(),iPlr);
}