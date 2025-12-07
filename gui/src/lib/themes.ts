export const themes = [
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
];

export const getTheme = (): string => {
  return localStorage.getItem("theme") || "dark";
};

export const setTheme = (theme: string): void => {
  localStorage.setItem("theme", theme);
  // Force theme update by removing and re-adding attribute
  document.documentElement.removeAttribute("data-theme");
  // Use requestAnimationFrame to ensure DOM update
  requestAnimationFrame(() => {
    document.documentElement.setAttribute("data-theme", theme);
    // Force a reflow to ensure styles are applied
    document.documentElement.offsetHeight;
  });
};

export const initializeTheme = (): void => {
  const savedTheme = getTheme();
  document.documentElement.setAttribute("data-theme", savedTheme);
};
