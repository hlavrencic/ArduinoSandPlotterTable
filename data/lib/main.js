var DEBUG = false;

var requireConfig = {
  baseUrl: "/lib",
  paths: {
    wifiserv: "wifiserv",
    configParams: "configParams",
    axios: "axios",
    apiService: "mods/apiService", // Comentar en PROD
    apiServiceReal: "mods/apiService"
  },
  waitSeconds: 30
};

if(DEBUG){
  requireConfig.paths.wifiserv = "wifiservDummy";
  requireConfig.paths.apiService = "mods/apiServiceLocal";
}

if (window.location.host.indexOf("stackblitz.io") >= 0) {
  requireConfig.paths.wifiserv = "/lib/wifiservDummy";
}

require.config(requireConfig);

define(["binder", "viewModel", "cmp/initComponents"], (binder, viewModel) => {
  binder.bindKnockOut();
  viewModel.apply();
  binder.bindForms();
});
