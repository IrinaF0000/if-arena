type TelegramWebApp = {
  initData: string;
  ready: () => void;
  expand?: () => void;
};

declare global {
  interface Window {
    Telegram?: {
      WebApp?: TelegramWebApp;
    };
  }
}

export type TelegramBridge = {
  ready: () => void;
  getRawInitData: () => string;
};

export function createTelegramBridge(): TelegramBridge {
  const webApp = window.Telegram?.WebApp;

  return {
    ready: () => {
      webApp?.ready();
      webApp?.expand?.();
    },
    getRawInitData: () => {
      // Security rule:
      // Send raw initData to the backend for validation.
      // Do not trust initDataUnsafe on the frontend.
      return webApp?.initData ?? "";
    }
  };
}
