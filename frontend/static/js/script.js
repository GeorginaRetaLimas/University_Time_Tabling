document.addEventListener('DOMContentLoaded', () => {
    const generateBtn = document.getElementById('generate-btn');
    const statusMsg = document.getElementById('status-message');
    const resultsSection = document.getElementById('results-section');
    const timetableDisplay = document.getElementById('timetable-display');
    const periodSelect = document.getElementById('period-select');
    const periodsData = JSON.parse(document.getElementById('periods-data').textContent);
    const timeLimitSelect = document.querySelector('select:nth-of-type(1)');
    const loadingModal = document.getElementById('loading-modal');

    // Update Info Panel on Period Change
    function updatePeriodInfo() {
        const selectedPeriod = periodSelect.value;
        const info = periodsData[selectedPeriod];

        // Update Period Name
        const periodNames = {
            "may-aug": "Mayo - Agosto",
            "sept-dec": "Septiembre - Diciembre",
            "jan-apr": "Enero - Abril"
        };

        document.querySelector('.info-row:nth-child(1) span:nth-child(2)').textContent = periodNames[selectedPeriod] || selectedPeriod;

        // Update Semesters
        const badgesContainer = document.querySelector('.info-row:nth-child(2) div');
        badgesContainer.innerHTML = '';
        const semesters = Object.keys(info.cuatrimestres).sort((a, b) => a - b);
        semesters.forEach(sem => {
            const badge = document.createElement('span');
            badge.className = 'badge';
            badge.textContent = `${sem}°`;
            badgesContainer.appendChild(badge);
        });
    }

    if (generateBtn && statusMsg && periodSelect) {
        periodSelect.addEventListener('change', updatePeriodInfo);
        updatePeriodInfo();

        generateBtn.addEventListener('click', async () => {
            loadingModal.style.display = 'flex';
            statusMsg.textContent = "Generando horario...";
            statusMsg.style.color = "#fbbf24";

            const period = periodSelect.value;

            let timeout = 60;
            // Removed timeLimitSelect logic as it was removed from HTML
            // Default to 60s or use hardcoded value if needed

            try {
                const response = await fetch('/api/solve', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ period: period, timeout: timeout })
                });

                const data = await response.json();

                if (data.status === 'success') {
                    statusMsg.textContent = "Horario generado con éxito!";
                    statusMsg.style.color = "#4ade80";
                    renderTimetable(data.solution);
                    resultsSection.classList.remove('results-hidden');
                } else {
                    statusMsg.textContent = "Error: " + data.message;
                    statusMsg.style.color = "#f87171";
                }
            } catch (error) {
                statusMsg.textContent = "Error de conexión: " + error;
                statusMsg.style.color = "#f87171";
            } finally {
                loadingModal.style.display = 'none';
            }
        });
    }

    function renderTimetable(solution) {
        // Paleta de 7 colores vibrantes (uno por cada materia del grupo)
        const colorPalette = [
            { bg: '#FFE5E5', border: '#FF6B6B', text: '#C92A2A' },  // Rojo
            { bg: '#E3F2FD', border: '#42A5F5', text: '#1565C0' },  // Azul
            { bg: '#E8F5E9', border: '#66BB6A', text: '#2E7D32' },  // Verde
            { bg: '#FFF3E0', border: '#FFA726', text: '#E65100' },  // Naranja
            { bg: '#F3E5F5', border: '#AB47BC', text: '#6A1B9A' },  // Púrpura
            { bg: '#E0F2F1', border: '#26A69A', text: '#00695C' },  // Teal
            { bg: '#FCE4EC', border: '#EC407A', text: '#AD1457' }   // Rosa
        ];

        // Agrupar por Grupo ID
        const groups = {};

        solution.forEach(item => {
            if (!groups[item.group_id]) groups[item.group_id] = [];
            groups[item.group_id].push(item);
        });

        let html = '';

        Object.keys(groups).sort((a, b) => a - b).forEach(groupId => {
            const items = groups[groupId];
            const sem = Math.floor(groupId / 100); // Asumiendo convención de ID

            // Asignar colores únicos a cada materia de este grupo
            const uniqueCourses = [...new Set(items.map(i => i.course_id))];
            const courseColorMap = {};
            uniqueCourses.forEach((courseId, index) => {
                courseColorMap[courseId] = colorPalette[index % 7];
            });

            html += `<div class="card" style="margin-bottom: 48px; border-top: 4px solid var(--primary);">
                <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 24px;">
                    <h3 style="color: var(--primary); margin: 0;">
                        <i class="fas fa-users"></i> Grupo ${groupId}
                    </h3>
                    <span class="badge" style="font-size: 1rem;">Cuatrimestre ${sem}</span>
                </div>`;

            html += `<div style="overflow-x: auto;">
                <table class="timetable-grid">
                    <thead>
                        <tr>
                            <th style="width: 100px;">Horario</th>
                            <th>Lunes</th>
                            <th>Martes</th>
                            <th>Miércoles</th>
                            <th>Jueves</th>
                            <th>Viernes</th>
                        </tr>
                    </thead>
                    <tbody>`;

            const hours = [
                "7:00-7:55", "7:55-8:50", "8:50-9:45", "9:45-10:40",
                "11:10-12:05", "12:05-13:00", "13:00-13:55", "14:00-14:55", "14:55-15:50"
            ];

            const dayOffsets = [0, 100, 200, 300, 400];

            hours.forEach((timeRange, index) => {
                html += `<tr>
                    <td style="font-weight: 600; color: var(--text-gray); font-size: 0.8rem;">${timeRange}</td>`;

                for (let day = 0; day < 5; day++) {
                    const targetId = (index + 1) + dayOffsets[day];
                    const cellItems = items.filter(i => i.timeslot_id === targetId);

                    html += `<td style="vertical-align: top; height: 80px;">`;
                    if (cellItems.length > 0) {
                        cellItems.forEach(ci => {
                            const colors = courseColorMap[ci.course_id];
                            html += `<div class="session-block" style="background: ${colors.bg}; border-left: 3px solid ${colors.border}; padding: 6px 8px; margin-bottom: 4px; border-radius: 4px; box-shadow: 0 1px 2px rgba(0,0,0,0.05);">
                                <div style="font-weight: 700; color: ${colors.text}; font-size: 0.85rem;">${ci.course_name}</div>
                                <div style="color: var(--text-gray); font-size: 0.75rem; margin-top: 2px;">
                                    <i class="fas fa-chalkboard-teacher"></i> ${ci.professor_name}
                                </div>
                            </div>`;
                        });
                    }
                    html += `</td>`;
                }
                html += `</tr>`;
            });

            html += `</tbody></table></div></div>`;
        });

        timetableDisplay.innerHTML = html;
    }

});
