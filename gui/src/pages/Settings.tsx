import { Card } from '../components/Card';
import { useState, useEffect } from 'react';
import { themes, getTheme, setTheme } from '../lib/themes';
import { Sun, Moon } from 'lucide-react';

export function Settings() {
  const [currentTheme, setCurrentTheme] = useState(getTheme());
  const [defaultEncrypt, setDefaultEncrypt] = useState(localStorage.getItem('defaultEncrypt') || 'aes-256-gcm');
  const [defaultHash, setDefaultHash] = useState(localStorage.getItem('defaultHash') || 'sha256');
  const [defaultCompression, setDefaultCompression] = useState(localStorage.getItem('defaultCompression') || 'lzma');
  const [defaultKdf, setDefaultKdf] = useState(localStorage.getItem('defaultKdf') || 'argon2id');

  useEffect(() => {
    setTheme(currentTheme);
  }, [currentTheme]);

  useEffect(() => {
    localStorage.setItem('defaultEncrypt', defaultEncrypt);
  }, [defaultEncrypt]);

  useEffect(() => {
    localStorage.setItem('defaultHash', defaultHash);
  }, [defaultHash]);

  useEffect(() => {
    localStorage.setItem('defaultCompression', defaultCompression);
  }, [defaultCompression]);

  useEffect(() => {
    localStorage.setItem('defaultKdf', defaultKdf);
  }, [defaultKdf]);

  const handleThemeChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    setCurrentTheme(e.target.value);
    // Blur to close dropdown (like Angular example)
    e.target.blur();
  };

  const toggleLightDark = () => {
    const newTheme = currentTheme === 'light' ? 'dark' : 'light';
    setCurrentTheme(newTheme);
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Settings</h1>
        <p className="text-base-content/70 mt-2">
          Configure FileVault application preferences
        </p>
      </div>

      <Card title="Theme">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Select Theme</label>
            <div className="flex gap-4">
              <select
                value={currentTheme}
                onChange={handleThemeChange}
                className="select select-bordered flex-1"
              >
                {themes.map((theme) => (
                  <option key={theme} value={theme}>
                    {theme.charAt(0).toUpperCase() + theme.slice(1)}
                  </option>
                ))}
              </select>
              <button
                onClick={toggleLightDark}
                className="btn btn-square btn-outline"
                title={currentTheme === 'light' ? 'Switch to Dark' : 'Switch to Light'}
              >
                {currentTheme === 'light' ? <Moon size={20} /> : <Sun size={20} />}
              </button>
            </div>
          </div>
        </div>
      </Card>

      <Card title="Default Algorithms">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Default Encryption Algorithm</label>
            <select
              value={defaultEncrypt}
              onChange={(e) => setDefaultEncrypt(e.target.value)}
              className="select select-bordered w-full"
            >
              <option value="aes-256-gcm">AES-256-GCM (Recommended)</option>
              <option value="aes-192-gcm">AES-192-GCM</option>
              <option value="aes-128-gcm">AES-128-GCM</option>
              <option value="chacha20-poly1305">ChaCha20-Poly1305</option>
              <option value="serpent-256-gcm">Serpent-256-GCM</option>
              <option value="twofish-256-gcm">Twofish-256-GCM</option>
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Default Hash Algorithm</label>
            <select
              value={defaultHash}
              onChange={(e) => setDefaultHash(e.target.value)}
              className="select select-bordered w-full"
            >
              <option value="sha256">SHA-256 (Recommended)</option>
              <option value="sha512">SHA-512</option>
              <option value="sha3-256">SHA3-256</option>
              <option value="blake2b-512">BLAKE2b-512</option>
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Default Compression</label>
            <select
              value={defaultCompression}
              onChange={(e) => setDefaultCompression(e.target.value)}
              className="select select-bordered w-full"
            >
              <option value="lzma">LZMA (Best Compression)</option>
              <option value="zlib">ZLIB (Balanced)</option>
              <option value="bzip2">BZIP2</option>
              <option value="none">None</option>
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Default KDF</label>
            <select
              value={defaultKdf}
              onChange={(e) => setDefaultKdf(e.target.value)}
              className="select select-bordered w-full"
            >
              <option value="argon2id">Argon2id (Recommended)</option>
              <option value="argon2i">Argon2i</option>
              <option value="pbkdf2-sha512">PBKDF2-SHA512</option>
              <option value="scrypt">Scrypt</option>
            </select>
          </div>
        </div>
      </Card>

      <Card title="Security">
        <div className="space-y-4">
          <label className="flex items-center gap-2">
            <input type="checkbox" className="checkbox checkbox-primary" defaultChecked />
            <span className="text-sm">Always verify signatures when decrypting</span>
          </label>
          <label className="flex items-center gap-2">
            <input type="checkbox" className="checkbox checkbox-primary" defaultChecked />
            <span className="text-sm">Secure delete temporary files</span>
          </label>
          <label className="flex items-center gap-2">
            <input type="checkbox" className="checkbox checkbox-primary" />
            <span className="text-sm">Automatically clear passwords after operation</span>
          </label>
        </div>
      </Card>

      <Card title="About">
        <div className="space-y-2">
          <p><strong>FileVault GUI</strong></p>
          <p className="text-sm text-base-content/70">Version 1.0.0</p>
          <p className="text-sm text-base-content/70">
            Secure file encryption and management tool
          </p>
          <p className="text-sm text-base-content/70 mt-4">
            Built with Tauri, React, and TypeScript
          </p>
        </div>
      </Card>
    </div>
  );
}
