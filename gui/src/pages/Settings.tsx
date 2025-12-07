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
    window.dispatchEvent(new CustomEvent('defaultsChanged'));
  }, [defaultEncrypt]);

  useEffect(() => {
    localStorage.setItem('defaultHash', defaultHash);
    window.dispatchEvent(new CustomEvent('defaultsChanged'));
  }, [defaultHash]);

  useEffect(() => {
    localStorage.setItem('defaultCompression', defaultCompression);
    window.dispatchEvent(new CustomEvent('defaultsChanged'));
  }, [defaultCompression]);

  useEffect(() => {
    localStorage.setItem('defaultKdf', defaultKdf);
    window.dispatchEvent(new CustomEvent('defaultsChanged'));
  }, [defaultKdf]);

  const handleThemeChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    setCurrentTheme(e.target.value);
    // Blur to close dropdown (like Angular example)
    e.target.blur();
  };

  const toggleLightDark = () => {
    const newTheme = currentTheme === 'filevault' ? 'filevaultDark' : 'filevault';
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
              <optgroup label="Symmetric GCM (Recommended)">
                <option value="aes-256-gcm">AES-256-GCM (Recommended)</option>
                <option value="aes-192-gcm">AES-192-GCM</option>
                <option value="aes-128-gcm">AES-128-GCM</option>
                <option value="chacha20-poly1305">ChaCha20-Poly1305 (Fast)</option>
                <option value="serpent-256-gcm">Serpent-256-GCM</option>
                <option value="twofish-256-gcm">Twofish-256-GCM</option>
                <option value="twofish-192-gcm">Twofish-192-GCM</option>
                <option value="twofish-128-gcm">Twofish-128-GCM</option>
                <option value="camellia-256-gcm">Camellia-256-GCM</option>
                <option value="camellia-192-gcm">Camellia-192-GCM</option>
                <option value="camellia-128-gcm">Camellia-128-GCM</option>
                <option value="aria-256-gcm">ARIA-256-GCM</option>
                <option value="aria-192-gcm">ARIA-192-GCM</option>
                <option value="aria-128-gcm">ARIA-128-GCM</option>
                <option value="sm4-gcm">SM4-GCM (Chinese Standard)</option>
              </optgroup>
              <optgroup label="AES Other Modes">
                <option value="aes-256-cbc">AES-256-CBC</option>
                <option value="aes-192-cbc">AES-192-CBC</option>
                <option value="aes-128-cbc">AES-128-CBC</option>
                <option value="aes-256-ctr">AES-256-CTR</option>
                <option value="aes-192-ctr">AES-192-CTR</option>
                <option value="aes-128-ctr">AES-128-CTR</option>
                <option value="aes-256-cfb">AES-256-CFB</option>
                <option value="aes-192-cfb">AES-192-CFB</option>
                <option value="aes-128-cfb">AES-128-CFB</option>
                <option value="aes-256-ofb">AES-256-OFB</option>
                <option value="aes-192-ofb">AES-192-OFB</option>
                <option value="aes-128-ofb">AES-128-OFB</option>
                <option value="aes-256-ecb">AES-256-ECB</option>
                <option value="aes-192-ecb">AES-192-ECB</option>
                <option value="aes-128-ecb">AES-128-ECB</option>
                <option value="aes-256-xts">AES-256-XTS</option>
                <option value="aes-128-xts">AES-128-XTS</option>
              </optgroup>
              <optgroup label="Legacy">
                <option value="3des">3DES (Legacy)</option>
              </optgroup>
              <optgroup label="Asymmetric">
                <option value="rsa-4096">RSA-4096 (Asymmetric)</option>
                <option value="rsa-3072">RSA-3072 (Asymmetric)</option>
                <option value="rsa-2048">RSA-2048 (Asymmetric)</option>
                <option value="ecc-p521">ECC-P521 (Asymmetric)</option>
                <option value="ecc-p384">ECC-P384 (Asymmetric)</option>
                <option value="ecc-p256">ECC-P256 (Asymmetric)</option>
              </optgroup>
              <optgroup label="Post-Quantum">
                <option value="kyber-1024-hybrid">Kyber-1024-Hybrid (Post-Quantum)</option>
                <option value="kyber-768-hybrid">Kyber-768-Hybrid (Post-Quantum)</option>
                <option value="kyber-512-hybrid">Kyber-512-Hybrid (Post-Quantum)</option>
              </optgroup>
              <optgroup label="Classical (Educational - Weak)">
                <option value="caesar">Caesar (Classical - Weak)</option>
                <option value="vigenere">Vigenere (Classical - Weak)</option>
                <option value="playfair">Playfair (Classical - Weak)</option>
                <option value="substitution">Substitution (Classical - Weak)</option>
                <option value="hill">Hill (Classical - Weak)</option>
              </optgroup>
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Default Hash Algorithm</label>
            <select
              value={defaultHash}
              onChange={(e) => setDefaultHash(e.target.value)}
              className="select select-bordered w-full"
            >
              <optgroup label="SHA-2 Family (Recommended)">
                <option value="sha256">SHA-256 (Recommended)</option>
                <option value="sha512">SHA-512</option>
                <option value="sha384">SHA-384</option>
                <option value="sha224">SHA-224</option>
                <option value="sha512-256">SHA-512/256</option>
              </optgroup>
              <optgroup label="SHA-3 Family">
                <option value="sha3-224">SHA3-224</option>
                <option value="sha3-256">SHA3-256</option>
                <option value="sha3-384">SHA3-384</option>
                <option value="sha3-512">SHA3-512</option>
              </optgroup>
              <optgroup label="BLAKE2 Family">
                <option value="blake2b-256">BLAKE2b-256</option>
                <option value="blake2b-384">BLAKE2b-384</option>
                <option value="blake2b-512">BLAKE2b-512</option>
                <option value="blake2s-256">BLAKE2s-256</option>
              </optgroup>
              <optgroup label="Legacy (Weak)">
                <option value="sha1">SHA-1 (Legacy - Weak)</option>
                <option value="md5">MD5 (Legacy - Weak)</option>
              </optgroup>
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
