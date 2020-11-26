class WifiStatusViewModel {
  static WifiStatus = {
    WL_IDLE_STATUS: 0,
    WL_NO_SSID_AVAIL: 1,
    WL_SCAN_COMPLETED: 2,
    WL_CONNECTED: 3,
    WL_CONNECT_FAILED: 4,
    WL_CONNECTION_LOST: 5,
    WL_DISCONNECTED: 6
  };

  static WifiConnectionStatus = {
    NONE: 0,
    DISCONNECTED: 1,
    CONNECTING: 2,
    CONNECTED: 3
  };

  static StatusName(code) {
    switch (code) {
      case 0:
        return "NONE";
      case 1:
        return "DISCONNECTED";
      case 2:
        return "CONNECTING";
      case 3:
        return "CONNECTED";
      default:
        return "";
    }
  }

  syncInterval = 0;
  pollTimeout;

  constructor(ko, apiService, blocker) {
    this.ko = ko;
    this.apiService = apiService;
    this.blocker = blocker;

    this.wifiStatus = ko.observable();
    this.status = ko.observable();
    this.wifiStatusTxt = ko.observable();
    this.ip = ko.observable();
    this.gw = ko.observable();
    this.mask = ko.observable();
    this.reason = ko.observable();
    this.aid = ko.observable();
    this.ssid = ko.observable();
    this.channel = ko.observable();

    let self = this;
    self.autoSync = ko.observable();
    self.autoSync.subscribe(value => {
      if (value) {
        self.initPoll();
      } else {
        if (self.pollTimeout) {
          clearTimeout(self.pollTimeout);
        }
      }
    });

    self.status.subscribe(status =>
      self.wifiStatusTxt(WifiStatusViewModel.StatusName(status))
    );
  }

  waitConnection(cancellationToken) {
    let self = this;

    let prom = new Promise((resolve, reject) => {
      self.initPoll(1000, wifi => {
        let status = wifi.status;

        if (
          (cancellationToken && cancellationToken.cancel) ||
          status == WifiStatusViewModel.WifiConnectionStatus.CONNECTED ||
          status == WifiStatusViewModel.WifiConnectionStatus.DISCONNECTED
        ) {
  
          if (status == WifiStatusViewModel.WifiConnectionStatus.CONNECTED) {
            resolve();
          } else if (status == WifiStatusViewModel.WifiConnectionStatus.DISCONNECTED) {
            reject(wifi.reason);
          }

          return false;
        }

        return true;
      }).then(() => {
        self.initPoll();
      });
    });

    return prom;
  }

  initPoll(interval, callback) {
    let self = this;

    if (self.pollTimeout) {
      clearTimeout(self.pollTimeout);
    }

    if (!interval) {
      self.syncInterval = 5000;
    } else {
      self.syncInterval = interval;
    }

    if(!callback) callback = wifi => true;

    self.autoSync(true);

    let prom = self
      .getData()
      .then(callback)
      .then(continuar => {
        return new Promise(resolve => {
          self.pollTimeout = setTimeout(() => resolve(continuar), self.syncInterval);
        });
      })
      .then(continuar => {
        if(!continuar) return;
        return self.initPoll(self.syncInterval, callback);
      });

    return prom;
  }

  getData() {
    let self = this;

    let rta = self.apiService
      .post("wifiStatus")
      .then(wifi => {
        self.ip(wifi.ip);
        self.gw(wifi.gw);
        self.mask(wifi.mask);
        self.reason(wifi.reason);
        self.aid(wifi.aid);
        self.ssid(wifi.ssid);
        self.channel(wifi.channel);
        self.wifiStatus(wifi.wifiStatus);
        self.status(wifi.status);
        return wifi;
      });

    return rta;
  }
}

define(["knockout", "apiService", "mods/blocker"], (
  ko,
  apiService,
  blocker
) => {
  return new WifiStatusViewModel(ko, apiService, blocker);
});
