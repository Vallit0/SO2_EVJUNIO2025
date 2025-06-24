import React, { useState, useEffect } from 'react';
import Chart from "chart.js/auto";
import { Line } from 'react-chartjs-2';

const PagefaultChart = () => {
    const API_URL = import.meta.env.VITE_API_URL
    const ENDPOINT_PAGEFAULT = import.meta.env.VITE_ENDPOINT_PAGEFAULT
    
    /**
     * timestamp
     * x -> formatted timestamp
     * y -> value
     * [
     *     {timestamp: timestamp, x: string, y: int},
     *    {...}
     * ]
     **/
    const [minorfData, setMinorfData] = useState([]);
    const [majorfData, setMajorfData] = useState([]);


    const getTimestamp = () => {
        return new Date().getTime();
    }

    const formatTimestamp = (timestamp) => {
        // Crear un objeto Date a partir del timestamp
        const date = new Date(timestamp);
    
        // Obtener las horas, minutos y segundos
        const hours = String(date.getUTCHours()).padStart(2, '0');
        const minutes = String(date.getUTCMinutes()).padStart(2, '0');
        const seconds = String(date.getUTCSeconds()).padStart(2, '0');
    
        // Formatear como HH:MM:SS
        return `${hours}:${minutes}:${seconds}`;
    }

    const getFiveRecents = (data) => {
        return data.slice(Math.max(data.length - 5, 0));
    }

    const updatePagefaultData = async () => {
        const requestOptions = {
            method: 'GET',
        }
        const response = await fetch(API_URL + ENDPOINT_PAGEFAULT, requestOptions);
        const data = await response.json();
        const timestamp = getTimestamp();

        let fmtMinorfData = {
            timestamp: timestamp,
            x: formatTimestamp(timestamp),
            y: data.minorf_vms,
        }

        let fmtMajorfData = {
            timestamp: timestamp,
            x: formatTimestamp(timestamp),
            y: data.majorf_vms,
        }

        const arrayMinorf = minorfData;
        arrayMinorf.push(fmtMinorfData);

        const arrayMajorf = majorfData;
        arrayMajorf.push(fmtMajorfData);
        
        arrayMinorf.sort((a, b) => a.timestamp - b.timestamp);
        arrayMajorf.sort((a, b) => a.timestamp - b.timestamp);

        const newMinorfData = getFiveRecents(arrayMinorf);
        const newMajorfData = getFiveRecents(arrayMajorf);

        setMinorfData(newMinorfData);
        setMajorfData(newMajorfData);
    }

    useEffect(() => {
        // Esta función se ejecutará cada 5000 milisegundos (5 segundos)
        const intervalId = setInterval(() => {
            updatePagefaultData();
        }, 2000);

        // Limpieza del intervalo cuando el componente se desmonta o actualiza
        return () => clearInterval(intervalId);
    }, []);

    const minorfDataset = {
        labels: minorfData.map((d) => d.x),
        datasets: [{
            label: 'Fallos de página',
            data: minorfData.map((d) => d.y),
            borderColor: 'rgba(75, 192, 192, 1)',
            borderWidth: 2,
            fill: false,
        }],
    }

    const majorfDataset = {
        labels: majorfData.map((d) => d.x),
        datasets: [{
            label: 'Fallos de página',
            data: majorfData.map((d) => d.y),
            borderColor: 'rgba(192, 75, 75, 1)',
            borderWidth: 2,
            fill: false,
        }],
    }

    return (
        <div className='container mx-auto py-5 h-full'>
            <h1 className="text-center text-5xl my-8">Grafica de lineas para fallos de página</h1>

            <div className="flex flex-col w-full">
                <div className="grid h-auto flex-grow card bg-base-300 rounded-box place-items-center py-5">
                    <h2 className="text-center text-3xl mb-2">Fallos de página menores</h2>
                    <div className="py-5 px-5 w-10/12">
                        <Line data={minorfDataset} height={"25rem"} width={"100%"} />
                    </div>
                </div>

                <div className="divider"></div>

                <div className="grid h-auto flex-grow card bg-base-300 rounded-box place-items-center py-5">
                    <h2 className="text-center text-3xl mb-2">Fallos de página mayores</h2>
                    <div className="py-5 px-5 w-10/12">
                        <Line data={majorfDataset} height={"25rem"} width={"100%"} />
                    </div>
                </div>
            </div>
        </div>
    )
}

export default PagefaultChart;