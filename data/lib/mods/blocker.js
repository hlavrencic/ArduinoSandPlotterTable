class Blocker {
  constructor(jquery) {
    this.jquery = jquery;
  }

  on(message) {
    if (!message) {
      message = '<div class="spinner"></div>';
    }

    this.jquery.blockUI({ message });
    this.jquery(".blockOverlay")
      .attr("title", "Click to unblock")
      .click($.unblockUI);
  }

  loading(onClick) {
    if (!onClick) {
      this.jquery.blockUI({ message: '<div class="spinner"></div>' });
    } else {
      this.jquery.blockUI({ message: '<div class="spinner secondary"></div>' });
      this.jquery(".blockOverlay")
        .attr("title", "Click to unblock")
        .click(() => {
          onClick();
          $.unblockUI();
        });
    }
  }

  off() {
    this.jquery.unblockUI();
  }
}

define(["jquery", "blockUI"], ($, blockUI) => new Blocker($));
