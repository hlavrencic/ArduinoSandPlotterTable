class NotificationBar {
  ko;
  connectionStatus;

  constructor(ko, apiService, blocker, wifiStatus) {
    this.ko = ko;
    this.apiService = apiService;
    this.blocker = blocker;

    this.console = ko.observableArray();
    this.socketResponsesAlert = ko.observable(false);
    this.wifiStatusLogo = ko.observable();
    this.showConsole = ko.observable(false);
    this.showWifi = ko.observable(false);
    this.wifi = ko.observable(wifiStatus);
  }

  init() {
    let self = this;

    let wifi = self.wifi();
    wifi.autoSync(true);

    wifi.status.subscribe(s => {
      let nro = 0;
      switch (s) {
        case WifiStatusViewModel.WifiConnectionStatus.CONNECTING:
          nro = 10067;
          break;
        case WifiStatusViewModel.WifiConnectionStatus.CONNECTED:
          nro = 9989;
          break;
        case WifiStatusViewModel.WifiConnectionStatus.DISCONNECTED:
          nro = 9940;
          break;
        default:
          nro = 8987;
      }

      let txt = String.fromCharCode(nro);
      self.wifiStatusLogo(txt);
    });
  }

  autoSyncText() {
    let rta = "Sync: ";
    if (this.wifi().autoSync()) rta += "ON";
    else rta += "OFF";
    return rta;
  }

  subscribeSocket(viewModel) {
    let self = this;

    viewModel.lastResponse.subscribe(r => {
      self._addConsole(1, JSON.stringify(r));
    });

    viewModel.sentData.subscribe(sent => {
      self._addConsole(2, JSON.stringify(sent));
    });

    viewModel.error.subscribe(error => {
      self._addConsole(3, error);
    });

    self.socketResponsesAlert.subscribe(v =>
      setTimeout(() => {
        if (self.socketResponsesAlert()) self.socketResponsesAlert(false);
      }, 1000)
    );

    self.connectionStatus = viewModel.connectionStatus;
  }

  getLocation() {
    return window.location.pathname;
  }

  consoleTypeSymbol(type) {
    switch (type) {
      case 1:
        return "↘";
      case 2:
        return "↖";
      default:
        return "";
    }
  }

  _addConsole(type, data) {
    this.console.push({
      time: new Date(),
      data: data,
      type: type
    });
  }
}

define([
  "knockout",
  "viewModel",
  "apiService",
  "mods/blocker",
  "mods/wifiStatus"
], (ko, viewModel, apiService, blocker, wifiStatus) => {
  ko.components.register("notification-bar", {
    viewModel: {
      createViewModel: (params, componentInfo) => {
        let model = new NotificationBar(ko, apiService, blocker, wifiStatus);
        model.subscribeSocket(viewModel);
        model.init();
        return model;
      }
    },
    template: { require: "text!cmp/notificationBar.html" }
  });
});
