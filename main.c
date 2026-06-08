#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Performans testi icin eklendi

// Yedek Parça Stok Yönetimi

/* Yedek parçlarımızı tutacağımız struct
  Her bir parçanın bir kimlik numarası , adı ve , stok adedi vardır
  Ayrıca solunda ve sağında kendisi gibi başka parçaları tutabilmesi için iki tane pointer var
  Sol tarafa her zaman kendinden küçük ID'li parçalar bağlanır.
  Sağ tarafa her zaman kendinden büyük ID'li parçalar bağlanır.
*/
typedef struct PartNode {
    int part_id;
    char name[50];
    int quantity;
    struct PartNode* left;
    struct PartNode* right;
} PartNode;

/*
  Bu fonksiyon yeni bir yedek parça kutusu oluşturmak için kullanılır.
  Bellekten yer ayırır , içine bizim gönderdiğimiz id, isim ve miktar bilgilerini koyar.
  Sağında ve solunda default olarak NULL değeri atanır.
*/
PartNode* createPart(int id, const char* name, int quantity) {
    PartNode* newNode = (PartNode*)malloc(sizeof(PartNode));
    newNode->part_id = id;
    strcpy(newNode->name, name);
    newNode->quantity = quantity;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

/*
  Ağaca yeni bir node ekleme fonksiyonu.
  Eğer ağaç tamamen boşsa, createPart fonksiyonunu çağırıp ilk parçayı ekler.
  Eğer ekleyeceğimiz id şu an baktığımız parçanın idsinden küçükse sol yoldan devam eder.
  Eğer ekleyeceğimiz id büyükse sağ yoldan devam eder.
  Eğer idler tamamen aynıysa, demek ki bu parça zaten sistemde var,o zaman sadece adetini arttırır.
*/
PartNode* insertPart(PartNode* root, int id, const char* name, int quantity) {
    if (root == NULL) return createPart(id, name, quantity);

    if (id < root->part_id)
        root->left = insertPart(root->left, id, name, quantity);
    else if (id > root->part_id)
        root->right = insertPart(root->right, id, name, quantity);
    else
        root->quantity += quantity;

    return root;
}

/*
  Ağaçtaki en küçük değeri bulma fonksiyonu.
  Biliyoruz ki en küçük değer her zaman ağacın en solundadır.
  Bu yüzden sola doğru gidebildiğimiz kadar gidip en sondaki düğümü buluyoruz.
  Bu fonksiyon birazdan parça silme işleminde işimize yarayacak.
*/
PartNode* minValueNode(PartNode* node) {
    PartNode* current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

/*
  Ağaçtan parça tamamen silme fonksiyonu. ( Stoğun 0'a inmesi durumu için )
  Önce silinecek parçayı arayıp buluruz
  Bulduğumuzda 3 ihtimal vardır:
  1- Hiç çocuğu yoktur: Direkt sileriz.
  2- Tek çocuğu vardır: Çocuğunu kendi yerine geçirir, kendisini sileriz.
  3- İki çocuğu vardır: Sağ taraftaki alt ağacın en küçük değerini bulup onun bilgilerini kopyalarız, sonra o kopyaladığımız alttaki değeri sileriz.
*/
PartNode* deletePartNode(PartNode* root, int id) {
    if (root == NULL) return root;

    if (id < root->part_id)
        root->left = deletePartNode(root->left, id);
    else if (id > root->part_id)
        root->right = deletePartNode(root->right, id);
    else {
        // Sol taraf boşsa sağdaki çocuğu al yerine koy
        if (root->left == NULL) {
            PartNode* temp = root->right;
            free(root);
            return temp;
        // Sağ taraf boşsa soldaki çocuğu al yerine koy
        } else if (root->right == NULL) {
            PartNode* temp = root->left;
            free(root);
            return temp;
        }
        // İki çocuk da varsa sağ tarafın en solundakini (en küçük olanı) bul
        PartNode* temp = minValueNode(root->right);
        // Onun bilgilerini buraya kopyala
        root->part_id = temp->part_id;
        strcpy(root->name, temp->name);
        root->quantity = temp->quantity;
        // Kopyaladığın o en alttaki eski düğümü temizle
        root->right = deletePartNode(root->right, temp->part_id);
    }
    return root;
}

/*
  Stoktaki yedek parçayı kullanıp adetini düşürme fonksiyonu.
  Önce idye göre parçayı arar.
  Bulursa ve stokta yeteri kadar varsa sayısını düşürür.
  Eğer sayıyı düşürdükten sonra stok 0 olursa, deletePartNode fonksiyonunu çağırarak parçayı sistemden siler.
*/
PartNode* usePart(PartNode* root, int id, int amount) {
    PartNode* current = root;
    while (current != NULL) {
        if (id == current->part_id) {
            // İstenen miktar kadar stok var mı diye bakıyoruz
            if (current->quantity >= amount) {
                current->quantity -= amount;
                printf("\n%d adet '%s' kullanildi. Kalan stok: %d\n", amount, current->name, current->quantity);

                // Eğer stok kalmadıysa parçayı ağaçtan komple silme
                if (current->quantity == 0) {
                    printf("Stoklar bitti , parca sistemden silinmistir\n");
                    return deletePartNode(root, id);
                }
                return root;
            } else {
                printf("\nYetersiz stok vardir . Mevcut miktar: %d\n", current->quantity);
                return root;
            }
        } else if (id < current->part_id) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    // Döngü bitti ama parça bulunamadıysa uyarı veriyoruz
    // Performans testinde arama işlemi yaparken ekrana bu hatayı basmasın diye yorum satırı yapılabilir
    // printf("\nHata: %d ID'li parca bulunamadi.\n", id);
    return root;
}

// Parçalar üzerinde inorder dolaşma
// Stok durumunu göstermekte işimize yarayacak
void inorderParts(PartNode* root) {
    if (root != NULL) {
        inorderParts(root->left);
        printf("Parca ID: %d  Ad: %s  Stok: %d\n", root->part_id, root->name, root->quantity);
        inorderParts(root->right);
    }
}

// Araç Kuyruğu Yönetimi

/*
 Kuyruktaki araçları tutmak için struct.
 Bu struct aracin modelini , problemini , ve sonraki aracı göstermek için pointer tutar.
*/
typedef struct CustomerNode {
    char vehicle_model[30];
    char issue[50];
    struct CustomerNode* next;
} CustomerNode;

/*
  Kuyruğun başını ve sonunu tutan asıl yapı.
  front: İşlem sırası gelen, en öndeki aracı gösterir.
  rear: Kuyruğa en son katılan, en arkadaki aracı gösterir.
*/
typedef struct Queue {
    CustomerNode *front, *rear;
} Queue;

/*
  Sistem açıldığında ilk başta bomboş bir kuyruk oluşturma fonksiyonu.
  Daha araç olmadığı için default olarak ön ve arkaya NULL değeri atanır
*/
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

/*
  Kuyruğun en arkasına yeni bir araç ekleme fonksiyonu (Enqueue).
  Önce yeni araç için hafızada yer ayırıyoruz. Sonra bilgilerini içine kopyalıyoruz.
  Eğer kuyruk zaten boşsa, bu araç hem ilk hem son araç oluyor.
  Değilse, kuyruğun en arkasındaki aracın arkasına bu yeni aracı ekleyip, en son aracı güncelliyoruz.
  isQuiet parametresi 1 gelirse ekrana "Eklendi" yazısı yazdırmaz (Dosyadan okurken ekran dolmasın diye).
*/
void enqueue(Queue* q, const char* model, const char* issue, int isQuiet) {
    CustomerNode* temp = (CustomerNode*)malloc(sizeof(CustomerNode));
    strcpy(temp->vehicle_model, model);
    strcpy(temp->issue, issue);
    temp->next = NULL; // Bu aracın arkasında henüz kimse yok

    // Eğer kuyruk bossa ilk araç budur
    if (q->rear == NULL) {
        q->front = q->rear = temp;
    } else {
        // Mevcut son aracın arkasına yeni aracı bağla
        q->rear->next = temp;
        // Artık son aracımız bu yeni araç oldu
        q->rear = temp;
    }

    // Ekrana sessizce ekle seçeneği kapalıysa mesaj ver
    if(!isQuiet) printf("\nArac kuyruga eklendi : %s\n", model);
}

/*
  Kuyruğun en önündeki aracı çağırıp kuyruktan çıkarma fonksiyonu
  Eğer kuyruk boşsa hiçbir şey yapmıyoruz.
  Kuyrukta araç varsa en öndeki aracı alıyoruz, ekrana yazdırıyoruz.
  Sonra ön sırayı bir arkadaki araca geçiriyoruz.
  En son da işi biten ilk aracı hafızadan temizliyoruz.
*/
void dequeue(Queue* q) {
    if (q->front == NULL) {
        printf("\nBekleyen arac yok. Servis su an bos.\n");
        return;
    }

    // En öndeki aracı geçici bir yere al
    CustomerNode* temp = q->front;
    printf("\n ISLEME ALINAN ARAC: %s  Islem: %s\n", temp->vehicle_model, temp->issue);

    // Kuyruğun önünü bir arkadaki araca kaydır
    q->front = q->front->next;

    // Eğer son aracı da çıkardıysak kuyruk tamamen boşalmıştır, arkayı da NULL yap
    if (q->front == NULL) q->rear = NULL;

    // Çıkarılan aracı hafızadan sil
    free(temp);
}

/*
  Mevcut kuyruğu baştan sona listeleme fonksiyonu.
  Kuyruk boşsa uyarı verir.
  Doluysa front'tan başlayıp next ile ilerleyerek son araca kadar ekrana yazdırır.
*/
void displayQueue(Queue* q) {
    if (q->front == NULL) {
        printf("\nKuyrukta bekleyen arac bulunmamaktadir.\n");
        return;
    }

    CustomerNode* temp = q->front;
    int sira = 1;

    printf("\nBEKLEYEN ARAC KUYRUGU\n");
    while (temp != NULL) {
        printf("%d. Siradaki Arac: %s  |  Islem: %s\n", sira, temp->vehicle_model, temp->issue);
        temp = temp->next;
        sira++;
    }
    printf("\n");
}

// Dosya yazma okuma işlemleri

// Ağaçtaki parçaları dosyaya sırayla yazdıran yardımcı fonksiyon.
void savePartsToFileHelper(PartNode* root, FILE* fp) {
    if (root != NULL) {
        // Dosyaya yazdırma işlemi
        fprintf(fp, "%d;%s;%d\n", root->part_id, root->name, root->quantity);
        savePartsToFileHelper(root->left, fp);
        savePartsToFileHelper(root->right, fp);
    }
}

/*
  Yedek parça dosyasını baştan oluşturup tüm ağacı içine kaydeder.
  write modu kullanıldığı için her seferinde dosyayı silip içindeki güncel verilerle baştan yazar.
*/
void updatePartsFile(PartNode* root) {
    FILE* fp = fopen("yedek_parca.txt", "w");
    if (fp) {
        savePartsToFileHelper(root, fp);
        fclose(fp);
    }
}

/*
  Program ilk açıldığında yedek_parca.txt dosyasını bulur, okur ve içindeki verilerle ağacı oluşturur.
  read moduyla açarız.Eğer dosya daha önce oluşturulmamışsa hiçbir şey yapmaz geri döner.
*/
PartNode* loadPartsFile(PartNode* root) {
    FILE* fp = fopen("yedek_parca.txt", "r");
    if (!fp) return root;

    int id, qty;
    char name[50];
    // Dosyayı okuma işlemleri
    while (fscanf(fp, "%d;%[^;];%d\n", &id, name, &qty) == 3) {
        root = insertPart(root, id, name, qty);
    }
    fclose(fp);
    return root;
}

/*
  Kuyruk dosyasını günceller. Kuyruğun en başından sonuna kadar tek tek giderek
  "Araç Modeli;Yapılacak İşlem" şeklinde dosyaya yazar.
*/
void updateQueueFile(Queue* q) {
    FILE* fp = fopen("arac_kuyrugu.txt", "w");
    if (fp) {
        CustomerNode* temp = q->front;
        // Kuyruk bitene kadar (temp NULL olana kadar) döngüye devam et
        while (temp) {
            fprintf(fp, "%s;%s\n", temp->vehicle_model, temp->issue);
            temp = temp->next; // Bir sonraki araca geç
        }
        fclose(fp);
    }
}

/*
  Program ilk açıldığında arac_kuyrugu.txt dosyasını okur.
  isQuiet = 1 olarak (ekrana yazı yazdırmamak için) kuyruğa araçları ekler.
*/
void loadQueueFile(Queue* q) {
    FILE* fp = fopen("arac_kuyrugu.txt", "r");
    if (!fp) return;

    char model[30], issue[50];
    while (fscanf(fp, "%[^;];%[^\n]\n", model, issue) == 2) {
        enqueue(q, model, issue, 1);
    }
    fclose(fp);
}

// Performans Testi
void performansTesti() {
    PartNode* testTree = NULL;
    clock_t baslangic, bitis;
    double dizi_suresi, bst_suresi;

    // Burdaki test boyutu raporda da belirtildiği gibi 100,1000,10000 boyutları ile test edilmiştir
    int test_boyutu = 10000;

    // Dizi için bellek ayırma
    int* testDizisi = (int*)malloc(test_boyutu * sizeof(int));

    printf("\n%d ADET PARCA ICIN PERFORMANS TESTI\n", test_boyutu);

    // Verileri hem Diziye hem de Agaca dolduruyoruz
    for (int i = 0; i < test_boyutu; i++) {
        int rastgele_id = rand() % 500000; // Karisik ID'ler
        testTree = insertPart(testTree, rastgele_id, "Test Parca", 1);
        testDizisi[i] = rastgele_id;
    }

    int aranan_id = -1;

    long tekrar_sayisi = 1000000;

    volatile int derleyici_kandirmacasi = 0;

   // dizi ile arama testi
    baslangic = clock();
    for(long k = 0; k < tekrar_sayisi; k++) {
        for(int i = 0; i < test_boyutu; i++) {
            if(testDizisi[i] == aranan_id) {
                derleyici_kandirmacasi++;
                break;
            }
        }
    }
    bitis = clock();
    dizi_suresi = ((double)(bitis - baslangic)) / CLOCKS_PER_SEC * 1000.0;

    // BST ile arama testi
    baslangic = clock();
    for(long k = 0; k < tekrar_sayisi; k++) {
        PartNode* current = testTree;
        while (current != NULL) {
            if (aranan_id == current->part_id) {
                derleyici_kandirmacasi++;
                break;
            }
            else if (aranan_id < current->part_id) current = current->left;
            else current = current->right;
        }
    }
    bitis = clock();
    bst_suresi = ((double)(bitis - baslangic)) / CLOCKS_PER_SEC * 1000.0;

    // Sonuclari Ekrana Bas
    printf("Dizi ile Arama Suresi: %.3f ms\n", dizi_suresi);
    printf("BST ile Arama Suresi  : %.3f ms\n", bst_suresi);

    free(testDizisi); // Diziyi bellekten temizle
}


int main() {
    // Ağaç ve kuyruğu oluşturma
    PartNode* inventory = NULL;
    Queue* serviceQueue = createQueue();

    // Dosya okuma işlemleri
    inventory = loadPartsFile(inventory);
    loadQueueFile(serviceQueue);

    int choice;
    char vehicleInput[30];
    char issueInput[50];
    int partId, quantity;
    char partName[50];

    // Menü ekranı
    while (1) {
        printf("\nOTO BAKIM SERVISI YONETIM SISTEMI\n");
        printf("1. Siraya arac ekleme\n");
        printf("2. Siradaki aracin islemlerine basla\n");
        printf("3. Mevcut kuyrugu goruntule\n");
        printf("4. Yedek parca ekleme\n");
        printf("5. Yedek parca eksiltme\n");
        printf("6. Yedek parca stok durumu\n");
        printf("7. Cikis\n");
        printf("8. Performans testi\n");
        printf("Seciminiz: ");

        // Kullanıcı hatalı giriş almamak için kontrol
        if (scanf("%d", &choice) != 1) {
            printf("Gecersiz giris sayi giriniz.\n");
            // Kullanıcının hatalı girdiği veriyi hafızadan temizliyoruz
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                printf("\nArac Modeli/Plakasi: ");
                scanf(" %[^\n]", vehicleInput);
                printf("Yapilacak Islem: ");
                scanf(" %[^\n]", issueInput);
                enqueue(serviceQueue, vehicleInput, issueInput, 0);

                // Dosya guncelleme
                updateQueueFile(serviceQueue);
                break;

            case 2:
                // Aracı kuyruktan çıkarma
                dequeue(serviceQueue);
                // Dosyayı güncelleme
                updateQueueFile(serviceQueue);
                break;

            case 3:
                // Kuyruğu görüntüleme fonksiyonunu çağır
                displayQueue(serviceQueue);
                break;

            case 4:
                printf("\nParca ID : ");
                scanf("%d", &partId);
                printf("Parca Adi: ");
                scanf(" %[^\n]", partName);
                printf("Eklenecek Miktar: ");
                scanf("%d", &quantity);

                // Ağaca yeni parçayı ekle ve ya olan parçanın miktarını arttırma
                inventory = insertPart(inventory, partId, partName, quantity);
                updatePartsFile(inventory); // Dosyayı güncelleme
                printf("Stok basariyla guncellendi.\n");
                break;

            case 5:
                printf("\nKullanilacak Parca ID: ");
                scanf("%d", &partId);
                printf("Kac adet kullanilacak: ");
                scanf("%d", &quantity);

                // Parçayı kullanma fonksiyonu
                inventory = usePart(inventory, partId, quantity);
                updatePartsFile(inventory); // Dosyayı güncelleme
                break;

            case 6:
                printf("\nMEVCUT YEDEK PARCA STOGU\n");
                // Ağaç eğer boşa uyarı gönderilecektir.
                if (inventory == NULL) printf("Stokta parca bulunmamaktadir.\n");

                else inorderParts(inventory);
                break;

            case 7:
                printf("\nCikis yapiliyor\n");
                // Verileri kaydetme
                updatePartsFile(inventory);
                updateQueueFile(serviceQueue);
                exit(0);

            case 8:
                performansTesti();
                break;

            default:
                // Kullanıcı 1 ile 8 arası dışında bir rakam girerse burası çalışır
                printf("\nHatali secim yaptiniz. Lutfen 1-8 arasi bir deger girin.\n");
        }
    }
    return 0;
}