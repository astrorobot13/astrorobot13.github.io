async function networkFirst(request) {
  try {
    const networkResponse = await fetch(request);
    if (networkResponse.ok) {
      const cache = await caches.open("MyCache_1");
      cache.put(request, networkResponse.clone());
    }
    return networkResponse;
  } catch (error) {
    const cachedResponse = await caches.match(request);
    return cachedResponse || Response.error();
  }
}
self.addEventListener("fetch", (event) => {
  if (["GET", "HEAD", "OPTIONS"].includes(event.request.method)]) {
    event.respondWith(networkFirst(event.request));
  }
});
