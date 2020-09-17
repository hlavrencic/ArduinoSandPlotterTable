class SvgReader{

  _reader;
  _circle;
  _container;
  _rangeSelector;
  _path;
  _newImageSubscribers = [];

  constructor(parentTag){
    let self = this;
    self._reader = new FileReader();
    self._reader.onloadend = () => {
      self._container.innerHTML= self._reader.result;
      let svg1 = self._container.getElementsByTagName("svg")[0];
      self.processImg(svg1);
    };
    self._circle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
    self._container = document.createElement("div");
    parentTag.appendChild(self._container);

    self._rangeSelector = document.createElement("input");
    self._rangeSelector.setAttribute('type', "range");
  }

  subscribe(readResponse){
    this._newImageSubscribers.push(readResponse);
  }

  bindInput(inputElem){
    let self = this;
    inputElem.addEventListener('input', function(event){
      let file = event.target.files[0];

      if (file && file.type=="image/svg+xml") {
        self._reader.readAsText(file);
      } else {
        alert("wrong file type or no file provided");
      } 
    });
    
  }

  processImg(svg1){
    let path = svg1.getElementsByTagName("path")[0];
    this._path = path;

    let bBox = svg1.getBBox();
    bBox.totalLength = path.getTotalLength();
    let circleRatio = (bBox.width + bBox.height) / 50;

    this._rangeSelector.setAttribute('max', path.getTotalLength());
    this._rangeSelector.setAttribute('step', path.getTotalLength()/1000);
    this._rangeSelector.value = 0;

    // create a circle
    this._circle.setAttribute("r", circleRatio);
    this._circle.setAttribute("fill", "red");

    // attach it to the container
    svg1.appendChild(this._circle);

    this._newImageSubscribers.forEach(readResponse => readResponse(bBox));
  }

  moveNext(){
    if(this._rangeSelector.valueAsNumber >= this._rangeSelector.max) return false;
    this._rangeSelector.stepUp();
    this.updateCircle();
    return true;
  }

  getPointAtLength(){
   
    let value = this._rangeSelector.valueAsNumber;
    let pos = this._path.getPointAtLength(value);
    return pos;
  }

  updateCircle(){
    let self = this;
    let pos = self.getPointAtLength();
    self._circle.setAttribute('cx', pos.x);
    self._circle.setAttribute('cy', pos.y);
  }

  bindRange(rangeParent){
    let self = this;

    rangeParent.appendChild(self._rangeSelector);

    self._rangeSelector.addEventListener('change', e => self.updateCircle());
  }
}

if (typeof module !== 'undefined' && typeof module.exports !== 'undefined')
module.exports = SvgReader; 
