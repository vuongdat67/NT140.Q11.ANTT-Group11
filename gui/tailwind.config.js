import daisyui from "daisyui";

/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {},
  },
  plugins: [
    // Remove @tailwindcss/forms as it conflicts with daisyUI
    // require("@tailwindcss/forms"),
    daisyui({
      themes: [
        "filevault",
        "filevaultDark",
        "light",
        "dark",
        "cupcake",
        "bumblebee",
        "emerald",
        "corporate",
        "synthwave",
        "retro",
        "cyberpunk",
        "valentine",
        "halloween",
        "garden",
        "forest",
        "aqua",
        "lofi",
        "pastel",
        "fantasy",
        "wireframe",
        "black",
        "luxury",
        "dracula",
      ],
      darkTheme: "filevaultDark",
      base: true,
      styled: true,
      utils: true,
      logs: false,
      rtl: false,
    }),
  ],
};
