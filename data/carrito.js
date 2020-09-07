class Carrito{
  _wifiServ;
  _svgRead;
  _bBox;
  _scale = {x: 1, y: 1};

  constructor(wifiServ, svgRead){
    let self = this;
    self._wifiServ = wifiServ;
    self._svgRead = svgRead;

    self._wifiServ.subscribe(json => {
      if(!(json.xPos && json.yPos)) return;
      if(!self.arrived(json)) return;

      self.sendNext();
    });

    self._svgRead.subscribe(bBox => {
      self._bBox = bBox;
      self.setScale();
      self.sendNext();
    });
  }

  arrived(json){
    let self = this;
    let point = self.getScaledPoint();
    return point.x == json.xPos && point.y == json.yPos;
  }

  getScaledPoint(){
    let self = this;

    let bBox = self._bBox;
    let point = self._svgRead.getPointAtLength();
    point.x -= bBox.x;
    point.y -= bBox.y;
    
    point.x *= self._scale.x;
    point.y *= self._scale.y;

    point.x = Math.round( point.x );
    point.y = Math.round( point.y );
    return point;
  }

  setScale(){
    let bBox = this._bBox;
    let x = bBox.width;
    let y = bBox.height;

    let realRatio = 4000/35000;
    let ratio = x / y ;
    if(ratio > 1){
      this._scale.x = 4000 / x;
      this._scale.y = (this._scale.x / realRatio) / ratio;
    } else {
      this._scale.y = 35000 / y;
      this._scale.x = (this._scale.y * realRatio) * ratio
    }

    console.warn(this._scale);
  }

  sendNext(){
    let self = this;
    if(!self._svgRead.moveNext()) return false;

    let point = self.getScaledPoint();
    let jsonSend = {xPos: "" + point.x, yPos: "" + point.y};
    self._wifiServ.send(jsonSend);
  }
}

if (typeof module !== 'undefined' && typeof module.exports !== 'undefined')
module.exports = Carrito; 