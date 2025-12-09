package mx.edu.itses.marg.arduino2;

import com.panamahitek.ArduinoException;
import com.panamahitek.PanamaHitek_Arduino;
import java.awt.BorderLayout;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFrame;
import javax.swing.JPanel;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

public class Arduino2 extends JFrame {

    PanamaHitek_Arduino arduino = new PanamaHitek_Arduino();

    // Series para graficar
    XYSeries serieDistancia = new XYSeries("Distancia Ultrasonico");
    XYSeries serieAngulo = new XYSeries("Ángulo Servo");

    int contador = 0;

    public Arduino2() {

        setTitle("Arduino2 - Gráfica en Vivo");
        setSize(900, 500);
        setLayout(new BorderLayout());
        setDefaultCloseOperation(EXIT_ON_CLOSE);

        // Crear dataset
        XYSeriesCollection dataset = new XYSeriesCollection();
        dataset.addSeries(serieDistancia);
        dataset.addSeries(serieAngulo);

        // Crear gráfica
        JFreeChart grafica = ChartFactory.createXYLineChart(
                "Lecturas desde Arduino",
                "Tiempo",
                "Valor leído",
                dataset
        );

        // Panel para gráfica
        ChartPanel chartPanel = new ChartPanel(grafica);
        add(chartPanel, BorderLayout.CENTER);

        // Aquí se reciben los datos
        SerialPortEventListener listener = new SerialPortEventListener() {
            @Override
            public void serialEvent(SerialPortEvent spe) {
                try {
                    if (arduino.isMessageAvailable()) {

                        String msg = arduino.printMessage().trim();
                        System.out.println("Dato recibido: " + msg);

                        String[] datos = msg.split(",");

                        if (datos.length == 2) {
                            try {
                                int distancia = Integer.parseInt(datos[0]);
                                int angulo = Integer.parseInt(datos[1]);

                                // Agregar datos a las series
                                serieDistancia.add(contador, distancia);
                                serieAngulo.add(contador, angulo);

                                contador++;

                            } catch (NumberFormatException e) {
                                System.out.println("Error de formato en datos, ignorando...");
                            }

                        } else {
                            System.out.println("Datos incompletos recibidos");
                        }

                    }
                } catch (SerialPortException | ArduinoException ex) {
                    Logger.getLogger(Arduino2.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
        };

        // Conexión a Arduino
        try {
            System.out.println("Conectando a Arduino...");
            arduino.arduinoRX("COM6", 9600, listener);
            System.out.println("✔ Conexión exitosa");
        } catch (ArduinoException | SerialPortException ex) {
            Logger.getLogger(Arduino2.class.getName()).log(Level.SEVERE, null, ex);
        }

        setVisible(true);
    }

    public static void main(String[] args) {
        new Arduino2();
    }
}
