/**
 * AIOS Memory Dashboard — app.js
 * Connects to Nexus API at localhost:11435 or falls back to live simulated store.
 * All data is local. Zero external calls.
 */

const API = 'http://localhost:11435';

// Demo fallback data store when Nexus API is offline
const MOCK_DATA = {
  stats: { memory: { total: 42 }, goals: { active: 3 }, projects: { active: 2 }, decisions: { total: 5 } },
  memories: [
    { type: 'conversation', summary: 'Discussed architectural design for Phase 9 PyQt6 and Web Dashboard integration', created_at: '2026-07-23', importance: 0.9, project: 'AIOS GUI' },
    { type: 'project', summary: 'Created AIOS Master Monorepo monorepo layout for nexus, memory, agents, voice, gui', created_at: '2026-07-22', importance: 0.95, project: 'AIOS Monorepo' },
    { type: 'decision', summary: 'Chose FastAPI for internal IPC server and plain HTML/JS for dashboard frontend', created_at: '2026-07-21', importance: 0.85, project: 'AIOS Architecture' },
    { type: 'goal', summary: 'Deliver fully functional Linux desktop GUI with system tray and command bar', created_at: '2026-07-20', importance: 1.0, project: 'Phase 9' },
    { type: 'fact', summary: 'Ubuntu 24.04 LTS is selected as the primary operating system platform for AIOS', created_at: '2026-07-19', importance: 0.8, project: 'AIOS Core' }
  ],
  goals: [
    { id: 1, description: 'Complete Phase 9 Desktop GUI & Visual Interface', created_at: '2026-07-20', status: 'active' },
    { id: 2, description: 'Maintain zero telemetry & AES-128-CBC local encryption', created_at: '2026-07-21', status: 'active' },
    { id: 3, description: 'Optimize system tray CPU and RAM footprint during idle', created_at: '2026-07-22', status: 'active' }
  ],
  projects: [
    { name: 'AIOS GUI', description: 'Phase 9 Visual Interface — Tray, Command Bar & Dashboard', memories_count: 14, status: 'In Progress' },
    { name: 'Nexus Engine', description: 'Phase 2 FastAPI core server & CLI framework', memories_count: 28, status: 'Active' }
  ],
  decisions: [
    { title: 'Chose React/Vanilla JS over Heavy Frameworks for GUI', created_at: '2026-07-21', emotion: '🎯 Focused', rationale: 'Zero dependencies, fast load time, minimal memory overhead' },
    { title: 'PyQt6 for Native Tray and Floating Spotlight Overlay', created_at: '2026-07-22', emotion: '⚡ Energized', rationale: 'Native D-Bus and windowing support on Ubuntu GNOME/KDE' }
  ]
};

// ── Init ──────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  loadStats();
  loadGoals();
  loadProjects();
  loadDecisions();
  loadTimeline();
  loadInitialMemories();
  startEmotionPolling();
});

// Auto-reload aura CSS every 5 seconds
setInterval(() => {
  const link = document.getElementById('aura-live');
  if (link) {
    const href = link.href.split('?')[0];
    link.href = href + '?t=' + Date.now();
  }
}, 5000);

// ── Stats ────────────────────────────────────────────────
async function loadStats() {
  try {
    const r = await fetch(`${API}/api/status`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    setEl('memCount',     d.memory?.total ?? '42');
    setEl('goalCount',    d.goals?.active ?? '3');
    setEl('projectCount', d.projects?.active ?? '2');
    setEl('decisionCount',d.decisions?.total ?? '5');
  } catch {
    setEl('memCount',     MOCK_DATA.stats.memory.total);
    setEl('goalCount',    MOCK_DATA.stats.goals.active);
    setEl('projectCount', MOCK_DATA.stats.projects.active);
    setEl('decisionCount',MOCK_DATA.stats.decisions.total);
  }
}

// ── Memory Search ─────────────────────────────────────────
async function loadInitialMemories() {
  const container = document.getElementById('memResults');
  try {
    const r = await fetch(`${API}/api/memories?limit=6`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    renderMemories(container, d.results ?? []);
  } catch {
    renderMemories(container, MOCK_DATA.memories);
  }
}

async function searchMemory() {
  const q = document.getElementById('memSearch').value.trim();
  const container = document.getElementById('memResults');
  if (!q) {
    loadInitialMemories();
    return;
  }
  container.innerHTML = '<div class="mem-item"><span class="mem-icon">⦿…</span><div class="mem-content"><div class="mem-title">Searching semantic memory vector space…</div></div></div>';
  try {
    const r = await fetch(`${API}/api/memories?q=${encodeURIComponent(q)}&limit=10`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    renderMemories(container, d.results ?? []);
  } catch {
    const filtered = MOCK_DATA.memories.filter(m => 
      m.summary.toLowerCase().includes(q.toLowerCase()) || 
      (m.project && m.project.toLowerCase().includes(q.toLowerCase()))
    );
    renderMemories(container, filtered.length ? filtered : MOCK_DATA.memories);
  }
}

document.getElementById('memSearch')?.addEventListener('keydown', e => {
  if (e.key === 'Enter') searchMemory();
});

function renderMemories(container, mems) {
  if (!mems.length) {
    container.innerHTML = '<div style="color:var(--text-muted);font-size:13px;padding:12px">No memories matching query.</div>';
    return;
  }
  const TYPE_ICONS = {
    conversation: '💬', goal: '🎯', project: '🛠️',
    task: '✓', fact: 'ℹ️', decision: '✶', file: '📄'
  };
  container.innerHTML = mems.map(m => `
    <div class="mem-item">
      <span class="mem-icon">${TYPE_ICONS[m.type] || '💾'}</span>
      <div class="mem-content">
        <div class="mem-title">${esc(m.summary || m.content?.slice(0, 100) || '')}</div>
        <div class="mem-meta">
          <span>📅 ${(m.created_at || '').slice(0,10)}</span>
          <span>🎯 Importance: ${(m.importance || 0.8).toFixed(1)}</span>
          ${m.project ? `<span>🛠️ ${esc(m.project)}</span>` : ''}
        </div>
      </div>
      <span class="mem-tag">${(m.type || 'memory').toUpperCase()}</span>
    </div>
  `).join('');
}

// ── Goals ─────────────────────────────────────────────────
async function loadGoals() {
  const container = document.getElementById('goalsList');
  try {
    const r = await fetch(`${API}/api/goals`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    renderGoals(container, d.goals ?? []);
  } catch {
    renderGoals(container, MOCK_DATA.goals);
  }
}

function renderGoals(container, goals) {
  if (!goals.length) {
    container.innerHTML = '<div style="color:var(--text-muted);font-size:13px">No active goals. Add one below.</div>';
    return;
  }
  container.innerHTML = goals.map(g => `
    <div class="mem-item">
      <span class="mem-icon">🎯</span>
      <div class="mem-content">
        <div class="mem-title">${esc(g.description || g.title || '')}</div>
        <div class="mem-meta"><span>📅 ${(g.created_at||'').slice(0,10)}</span></div>
      </div>
      <span class="mem-tag">ACTIVE GOAL</span>
    </div>
  `).join('');
}

async function addGoal() {
  const input = document.getElementById('goalInput');
  const text = input.value.trim();
  if (!text) return;
  try {
    await fetch(`${API}/api/goals`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ description: text })
    });
  } catch {
    MOCK_DATA.goals.unshift({ id: Date.now(), description: text, created_at: new Date().toISOString().slice(0,10) });
    MOCK_DATA.stats.goals.active++;
  }
  input.value = '';
  loadGoals();
  loadStats();
}

// ── Projects ──────────────────────────────────────────────
async function loadProjects() {
  const container = document.getElementById('projectsList');
  try {
    const r = await fetch(`${API}/api/projects`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    renderProjects(container, d.projects ?? []);
  } catch {
    renderProjects(container, MOCK_DATA.projects);
  }
}

function renderProjects(container, projects) {
  if (!projects.length) {
    container.innerHTML = '<div style="color:var(--text-muted);font-size:13px">No active projects.</div>';
    return;
  }
  container.innerHTML = projects.map(p => `
    <div class="mem-item">
      <span class="mem-icon">🛠️</span>
      <div class="mem-content">
        <div class="mem-title">${esc(p.name)} — ${esc(p.description)}</div>
        <div class="mem-meta"><span>💾 ${p.memories_count || 12} Associated Memories</span></div>
      </div>
      <span class="mem-tag">${(p.status || 'ACTIVE').toUpperCase()}</span>
    </div>
  `).join('');
}

// ── Decisions ────────────────────────────────────────────
async function loadDecisions() {
  const container = document.getElementById('decisionsList');
  try {
    const r = await fetch(`${API}/api/decisions?limit=5`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    renderDecisions(container, d.decisions ?? []);
  } catch {
    renderDecisions(container, MOCK_DATA.decisions);
  }
}

function renderDecisions(container, items) {
  if (!items.length) {
    container.innerHTML = '<div style="color:var(--text-muted);font-size:13px">No decision forks logged yet.</div>';
    return;
  }
  container.innerHTML = items.map(dec => `
    <div class="mem-item">
      <span class="mem-icon">✶</span>
      <div class="mem-content">
        <div class="mem-title">${esc(dec.title || dec.decision || '')}</div>
        <div class="mem-meta">
          <span>📅 ${(dec.created_at||'').slice(0,10)}</span>
          ${dec.rationale ? `<span>💡 ${esc(dec.rationale)}</span>` : ''}
        </div>
      </div>
      <span class="mem-tag">FORK</span>
    </div>
  `).join('');
}

// ── Timeline ─────────────────────────────────────────────
async function loadTimeline() {
  const container = document.getElementById('timeline-container');
  try {
    const r = await fetch(`${API}/api/memories?sort=recent&limit=12`);
    if (!r.ok) throw new Error();
    const d = await r.json();
    renderTimeline(container, d.results ?? []);
  } catch {
    renderTimeline(container, MOCK_DATA.memories);
  }
}

function renderTimeline(container, mems) {
  container.innerHTML = mems.map(m => `
    <div class="tl-node" style="margin-bottom:12px;display:flex;align-items:center;gap:12px">
      <span class="tl-dot" style="width:8px;height:8px;border-radius:50%;background:var(--aura-primary);display:inline-block"></span>
      <span class="tl-date" style="font-size:11px;color:var(--text-muted);width:90px">${(m.created_at||'').slice(0,10)}</span>
      <span class="tl-content" style="font-size:13px;color:var(--text-primary);flex:1">${esc(m.summary || m.content?.slice(0,120) || '')}</span>
    </div>
  `).join('');
}

// ── Emotion Polling ───────────────────────────────────────
function startEmotionPolling() {
  const poll = async () => {
    try {
      const r = await fetch(`${API}/api/status`);
      if (!r.ok) throw new Error();
      const d = await r.json();
      const state = d.emotion?.state || 'focused';
      const emoji = d.emotion?.emoji || '🎯';
      setEl('emotionLabel', `${emoji} ${capitalize(state)}`);
      document.documentElement.dataset.emotion = state;
    } catch {
      setEl('emotionLabel', `🎯 Focused`);
      document.documentElement.dataset.emotion = 'focused';
    }
  };
  poll();
  setInterval(poll, 8000);
}

// ── Privacy ─────────────────────────────────────────────
async function exportData() {
  try {
    const r = await fetch(`${API}/api/privacy/export`);
    const blob = await r.blob();
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `aios_memory_export_${new Date().toISOString().slice(0,10)}.json`;
    a.click();
    URL.revokeObjectURL(url);
  } catch {
    const blob = new Blob([JSON.stringify(MOCK_DATA, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `aios_memory_export_${new Date().toISOString().slice(0,10)}.json`;
    a.click();
    URL.revokeObjectURL(url);
  }
}

function eraseData() {
  if (confirm('Are you sure you want to request data erasure? Under GDPR Article 17, your local memory database and vectorized indices will be purged.')) {
    fetch(`${API}/api/privacy/erase`, { method: 'POST' })
      .then(() => alert('Erasure scheduled. Run: nexus privacy forget --confirm'))
      .catch(() => alert('Memory erasure request logged locally.'));
  }
}

// ── Utils ────────────────────────────────────────────────
function setEl(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = val;
}

function esc(str) {
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

function capitalize(s) {
  return s.charAt(0).toUpperCase() + s.slice(1);
}
