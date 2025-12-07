import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { Sidebar } from './components/Sidebar';
import { Dashboard } from './pages/Dashboard';
import { Encrypt } from './pages/Encrypt';
import { Decrypt } from './pages/Decrypt';
import { Hash } from './pages/Hash';
import { Keygen } from './pages/Keygen';
import { Sign } from './pages/Sign';
import { Verify } from './pages/Verify';
import { Stego } from './pages/Stego';
import { Archive } from './pages/Archive';
import { Compress } from './pages/Compress';
import { Benchmark } from './pages/Benchmark';
import { Info } from './pages/Info';
import { Dump } from './pages/Dump';
import { Settings } from './pages/Settings';
import { initializeTheme } from './lib/themes';
import './index.css';
import { useEffect } from 'react';

function App() {
  useEffect(() => {
    initializeTheme();
  }, []);
  return (
    <BrowserRouter>
      <div className="flex h-screen bg-base-100 text-base-content">
        <Sidebar />
        <main className="flex-1 overflow-y-auto p-8">
          <Routes>
            <Route path="/" element={<Dashboard />} />
            <Route path="/encrypt" element={<Encrypt />} />
            <Route path="/decrypt" element={<Decrypt />} />
            <Route path="/hash" element={<Hash />} />
            <Route path="/keygen" element={<Keygen />} />
            <Route path="/sign" element={<Sign />} />
            <Route path="/verify" element={<Verify />} />
            <Route path="/stego" element={<Stego />} />
            <Route path="/archive" element={<Archive />} />
            <Route path="/compress" element={<Compress />} />
            <Route path="/benchmark" element={<Benchmark />} />
            <Route path="/info" element={<Info />} />
            <Route path="/dump" element={<Dump />} />
            <Route path="/settings" element={<Settings />} />
          </Routes>
        </main>
      </div>
    </BrowserRouter>
  );
}

export default App;
