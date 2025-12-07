export const themes = [
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
];

export const getTheme = (): string => {
  return localStorage.getItem("theme") || "filevault";
};

export const setTheme = (theme: string): void => {
  localStorage.setItem("theme", theme);
  
  // Force theme update - similar to Angular example
  const element = document.activeElement as HTMLElement;
  if (element) {
    element.blur(); // Remove focus from the button
  }
  
  // Remove old theme first to force CSS recalculation
  const html = document.documentElement;
  html.removeAttribute("data-theme");
  
  // Use requestAnimationFrame to ensure DOM is ready
  requestAnimationFrame(() => {
    // Set new theme attribute
    html.setAttribute("data-theme", theme);
    
    // Force a reflow to ensure styles are applied immediately
    void html.offsetHeight;
    
    // Trigger a custom event for any components that need to react
    window.dispatchEvent(new CustomEvent('themechange', { detail: { theme } }));
  });
};

export const initializeTheme = (): void => {
  const savedTheme = getTheme();
  document.documentElement.setAttribute("data-theme", savedTheme);
};
