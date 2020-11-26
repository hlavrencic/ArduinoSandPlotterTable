define(["knockout"], ko => {
  ko.components.register("footer-git", {
    template: { require: "text!cmp/footerGit.html" }
  });
});
