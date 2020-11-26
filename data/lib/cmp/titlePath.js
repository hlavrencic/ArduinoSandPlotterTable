class TitlePath {
  constructor(params) {}

  getLocation() {
    return window.location.pathname;
  }
}

define(["knockout"], ko => {
  ko.components.register("title-path", {
    viewModel: TitlePath,
    template: { require: "text!cmp/titlePath.html" }
  });
});
