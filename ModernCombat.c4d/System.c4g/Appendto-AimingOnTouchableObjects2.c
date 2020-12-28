#strict 3
#appendto HZCK


public func Grab(object target, bool fGrab)
{
  if (!fGrab && GetEffect("DidGrab", this) && GetPlrCoreJumpAndRunControl(GetController()))
  {
    this->SetAction("Walk");
    if(this->~ReadyToSquatAim())
      this->~StartSquatAiming();
  }
  else
    return (_inherited(target, fGrab));
}
