import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App";
import "./index.css";

// Initialize theme BEFORE React renders (like Angular ngOnInit)
// This ensures daisyUI theme is applied immediately
const savedTheme = localStorage.getItem("theme") || "dark";
if (!document.documentElement.hasAttribute("data-theme")) {
  document.documentElement.setAttribute("data-theme", savedTheme);
}

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
);
