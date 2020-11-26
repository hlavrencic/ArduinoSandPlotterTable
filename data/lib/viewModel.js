define(["knockout", "wifiserv"], (ko, wifiServ) => {
  class ViewModel {
    connectionStatus = ko.observable();
    sentData = ko.observable();
    lastResponse = ko.observable();
    error = ko.observable();

    subscribe(wifiServ) {
      let self = this;
      wifiServ._status = val => self.connectionStatus(val);
      wifiServ._errorEvt = evt => self.error(evt);

      wifiServ.subscribe(obj => {
        self.lastResponse(obj);
      });

      self.sentData.subscribe(data => {
        wifiServ.send(data);
      });
    }

    apply() {
      ko.applyBindings(this);
    }
  }

  let viewModel = new ViewModel();
  viewModel.subscribe(wifiServ);
  return viewModel;
});
