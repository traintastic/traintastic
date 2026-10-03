const CACHE = 'traintastic-throttle-v1';
const ASSETS = [
  '/throttle',
  '/css/normalize.css',
  '/css/throttle.css',
  '/js/throttle.js',
  '/appicon.svg',
  '/appicon-256.png'
];

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE).then((cache) => cache.addAll(ASSETS)).then(() => self.skipWaiting())
  );
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys()
      .then((keys) => Promise.all(keys.filter((k) => k !== CACHE).map((k) => caches.delete(k))))
      .then(() => self.clients.claim())
  );
});

self.addEventListener('fetch', (event) => {
  if (event.request.method !== 'GET') { return; }
  const url = new URL(event.request.url);
  if (url.protocol !== 'http:' && url.protocol !== 'https:') { return; }
  event.respondWith(
    fetch(event.request)
      .then((response) => {
        const copy = response.clone();
        caches.open(CACHE).then((cache) => cache.put(event.request, copy)).catch(() => {});
        return response;
      })
      .catch(() => caches.match(event.request).then((cached) => cached || caches.match('/throttle')))
  );
});
